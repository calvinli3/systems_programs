#include "helpers.h"


int get_min_freelist(size_t blocksize) {
    if (blocksize <= LIST_1_MAX) {
        return 0;
    } else if (blocksize <= LIST_2_MAX) {
        return 1;
    } else if (blocksize <= LIST_3_MAX) {
        return 2;
    } else if (blocksize <= LIST_4_MAX) {
        return 3;
    }
}

sf_free_header* search_free_lists(int i, size_t totalsize, size_t req_size) {
    sf_free_header *current = NULL;

    while (i < 4) {
        if (seg_free_list[i].head == NULL)    //current list is empty
            i++;
        else {  //current list is not empty
            current = seg_free_list[i].head;    //start current at the head of the list
            while (current != NULL) {
                if ((current->header.block_size << 4) >= totalsize) {    //if current block is available to alloc
                    //remove block from freelist
                    remove_block(current);
                    size_t sizediff = (current->header.block_size << 4) - totalsize;
                    //set header and footer of allocated block
                    current->header.allocated = 1;
                    current->header.block_size = (totalsize >> 4);
                    sf_footer* footer_ptr = (sf_footer*)((char*)current+(current->header.block_size<<4) - (SF_FOOTER_SIZE/8));
                    footer_ptr->requested_size = req_size;
                    if (((16-(req_size%16))%16) != 0) {
                        current->header.padded = 1;
                        footer_ptr->padded = 1;
                    }
                    footer_ptr->allocated = 1;
                    footer_ptr->block_size = (totalsize>>4);
                    if (sizediff >= 32) { // if there is >32 bytes remaining, split
                        //split block and insert to free list
                        sf_free_header* split_header = split_block(current, totalsize, sizediff);
                        insert_to_freelist(split_header);
                    }
                    //return pointer to head of free block
                    return current;
                }
                else {  //current block size cannot satisfy total block size, get next block in free list
                    current = current->next;
                }
            }
            i++;    //list is out of elements; get next list
        }
    }
    return NULL;
}


void remove_block(sf_free_header* node) {
    size_t size = (node->header.block_size) << 4;
    int i = get_min_freelist(size);
    // NODE 1 <-----> NODE 2 <-----> NODE 3
    if (node->next == NULL && node->prev == NULL) {   //no prev, no next
        seg_free_list[i].head = NULL;
    }
    else if (node->prev == NULL && node->next != NULL) {   //no prev, next exists
        node->next->prev = NULL;
        seg_free_list[i].head = node->next;
    }
    else if (node->next == NULL && node->prev != NULL) {   //no next, prev exists
        node->prev->next = NULL;
    }
    else if (node->next != NULL && node->prev != NULL) {   //next and prev exist
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
}

sf_free_header* split_block(sf_free_header* current, size_t alloc_blocksize, size_t sizediff) {
    // set header and footer of free block
    sf_free_header* free_block = (sf_free_header*)((char*)current + alloc_blocksize);
    free_block->header.allocated = 0;
    free_block->header.block_size = (sizediff >> 4);
    sf_footer* free_footer = (sf_footer*)((char*)free_block+(free_block->header.block_size << 4)-SF_FOOTER_SIZE/8);
    free_footer->allocated = 0;
    free_footer->block_size = (sizediff>>4);
    // return free block
    return free_block;
}

void coalesce_back(sf_free_header* block_header) {
    // check if address of get_heap_start < block_header-8
    sf_footer* prev_footer_ptr = (sf_footer*)((char*)block_header-(SF_FOOTER_SIZE/8));
    if (get_heap_start() > (void*)prev_footer_ptr) {  //if address of heap start < address of prev footer, dont coal
        insert_to_freelist(block_header);
        return;
    }
    if (prev_footer_ptr->allocated == 1) {   //if footer is not free, dont coalesce
        return;
    }
    //get the header of the prev block, block_header footer, and total block_size
    sf_free_header* prev_header_ptr = (sf_free_header*)((char*)block_header - (prev_footer_ptr->block_size<<4));
    //remove previous block from list
    remove_block(prev_header_ptr);
    sf_footer* curr_footer_ptr = (sf_footer*)((char*)block_header+(block_header->header.block_size<<4) - (SF_FOOTER_SIZE/8));
    size_t t_blocksize = (prev_footer_ptr->block_size<<4) + (curr_footer_ptr->block_size<<4);
    //set the new blocksize to the prev_header and block_header footer
    prev_header_ptr->header.block_size = t_blocksize>>4;
    curr_footer_ptr->block_size = t_blocksize>>4;
    //add new block to list
    insert_to_freelist(prev_header_ptr);
}

void insert_to_freelist(sf_free_header* block_header) {
    size_t size = (block_header->header.block_size) << 4;
    int i = get_min_freelist(size);
    sf_free_header* temp = seg_free_list[i].head;
    seg_free_list[i].head = block_header;
    if (temp != NULL) {
        block_header->next = temp;
        temp->prev = block_header;
    }
}

int check_invalid_pointer(void* ptr) {  //return 1 if invalid pointer
// The pointer is NULL
    if (ptr == NULL) {
        return 1;
    }
// The header of the block is before heap_start or block ends after heap_end
    sf_free_header* block_header = (sf_free_header*)((char*)ptr - (SF_HEADER_SIZE/8));
    if (get_heap_start() > (void*)block_header) {
        return 1;
    }
// The alloc bit in the header or footer is 0
    sf_footer* footer_ptr = (sf_footer*)((char*)block_header+(block_header->header.block_size<<4)-(SF_FOOTER_SIZE/8));

    if ((block_header->header.allocated == 0) || (footer_ptr->allocated == 0)) {
        return 1;
    }
// The requested_size, block size, and padded bits do not make sense when
// put together. For example, if requested_size + 16 != block size, you know
// that the padded bit must be 1.
    if ((footer_ptr->requested_size+((SF_HEADER_SIZE+SF_FOOTER_SIZE)/8)) != (footer_ptr->block_size<<4)){
        if (footer_ptr->padded != 1) {
            return 1;
        }
    }
    if (footer_ptr->padded == 1) {
        if ((footer_ptr->requested_size+((SF_HEADER_SIZE+SF_FOOTER_SIZE)/8)) == (footer_ptr->block_size<<4)){
            return 1;
        }
    }
// The padded and alloc bits in the header and footer are inconsistent.
    if ((block_header->header.allocated) != (footer_ptr->allocated)) {
        return 1;
    }
    if ((block_header->header.padded) != (footer_ptr->padded)) {
        return 1;
    }
    return 0;   //return 0 if valid pointer
}

void coalesce_fwd(sf_free_header* block_header, sf_free_header* next_block) {
    // check if end of heap is < the next block's footer address
    sf_footer* next_block_footer = (sf_footer*)((char*)next_block+(next_block->header.block_size<<4) - (SF_FOOTER_SIZE/8));
    if (get_heap_end() < (void*)next_block_footer) {
        return;
    }
    //get next_block footer, and total block_size
    //remove next block from list
    remove_block(next_block);
    size_t t_blocksize = (block_header->header.block_size<<4) + (next_block_footer->block_size<<4);
    //set the new blocksize to the block_header header and next_block footer
    block_header->header.block_size = t_blocksize>>4;
    next_block_footer->block_size = t_blocksize>>4;
    //do not add to list.
    return;
}