/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
// #include "sfmm.h"
#include "helpers.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

void *sf_malloc(size_t size) {
    // determine if request size is valid (invalid is 0 or greater than available allocator space)
    if ((size <= 0) || (size > (PAGE_SZ*4)))
        sf_errno = EINVAL;

    // determine necessary block size: payload+padding+header+footer = total size
    size_t padding = ((16-(size%16))%16);
    size_t totalsize = size + padding+ ((SF_HEADER_SIZE + SF_FOOTER_SIZE)/8);
    // start searching in the minimum relevant list; if free block not found, search in upper lists
    int i = get_min_freelist(totalsize);
    // search the list
    sf_free_header* free_block_ptr = search_free_lists(i, totalsize, size);

    while (free_block_ptr == NULL) {    //if there is not a free block, keep doing sf_sbrk
    // if no free blocks in any list, sf sbrk -> returns the address of the new page
        sf_free_header* prev_break = (sf_free_header*)sf_sbrk();
        if(prev_break == (void*)-1){
            sf_errno = ENOMEM;
            return NULL;
        }

        // set header and footer of the new page blocks
        prev_break->header.allocated = 0;
        prev_break->header.block_size = (PAGE_SZ >> 4);
        prev_break->next = NULL;
        prev_break->prev = NULL;
        prev_break->header.padded = 0;
        sf_footer* footer_ptr = (sf_footer*)((char*)prev_break + PAGE_SZ - (SF_FOOTER_SIZE/8));
        footer_ptr->allocated = 0;
        footer_ptr->block_size = (PAGE_SZ >> 4);
        // attempt to coalesce backward after sf_sbrk (but not beyond sf_heap_start)
        coalesce_back(prev_break);
        // check again
        free_block_ptr = search_free_lists(i, totalsize, size);
    }
    // return the pointer to the payload
    return (void*)((char*)free_block_ptr+8);
}

void *sf_realloc(void *ptr, size_t size) {
    // first, check if pointer and size are valid
    if (check_invalid_pointer(ptr)) {   // 1 if invalid - abort
        abort();
    }
    // if size is 0, free and return null
    if (size == 0) {
        sf_free(ptr);
        return NULL;
    }
    size_t padding = ((16-(size%16))%16);
    size_t totalsize = size + padding + ((SF_HEADER_SIZE + SF_FOOTER_SIZE)/8);
    sf_free_header* header_ptr = (sf_free_header*)((char*)ptr-(SF_HEADER_SIZE/8));
    size_t header_block_size = header_ptr->header.block_size << 4;
    sf_footer* footer_ptr = (sf_footer*)((char*)header_ptr+(header_block_size)-(SF_FOOTER_SIZE/8));
    if (totalsize == footer_ptr->block_size<<4) {
        footer_ptr->requested_size = size;
        if (padding == 0) {
            header_ptr->header.padded = 0;
            footer_ptr->padded = 0;
        } else {
            header_ptr->header.padded = 1;
            footer_ptr->padded = 1;
        }
        return ptr;
    } else if ((header_ptr->header.block_size<<4) < totalsize) {     // realloc to larger block
        // 1: Call sf_malloc to obtain a larger block
        sf_free_header* new_block = (sf_free_header*)sf_malloc(size);
        if (new_block == NULL) {    // if sf_malloc returns null, sf_realloc also returns null
            return NULL;
        }
        // 2: Call memcpy to copy the data in block given by user to block rtn by sf_malloc
        memcpy(((char*)new_block+8), ptr, totalsize);

        // 3: Call sf_free on the block given by the user (coalescing if necessary)
        sf_free(ptr);
        // 4: Return the block given to you by sf_malloc to the user
        return (void*)new_block;
    // Realloc to smaller block
    } else {
        size_t sizediff = header_block_size - totalsize;
        // 1: can't split- it would create a splinter.
        if (sizediff < 32) {
            // update footer requested size, done
            footer_ptr->requested_size = size;
        }
        // 2: can split without creating a splinter.
        else if (sizediff >= 32) { // if there is >32 bytes remaining, split
            //split block, update headers, free the split block
            sf_free_header* split_header = split_block(header_ptr, totalsize, sizediff);
            // determine if next block can be coalesced.
            sf_free_header* next_block = (sf_free_header*)((char*)split_header+(split_header->header.block_size<<4));
            if (next_block->header.allocated == 0) {
                coalesce_fwd(split_header, next_block);  // combine blocks; this will just set header and footer blocksize
            }
            //set allocated bits
            sf_footer* ftr_ptr = (sf_footer*)((char*)split_header+(split_header->header.block_size<<4)-(SF_FOOTER_SIZE/8));
            split_header->header.allocated = 0;
            ftr_ptr->allocated = 0;
            // insert to free list
            insert_to_freelist(split_header);
            //update blocksize in current header/footer
            header_ptr->header.block_size =  totalsize>>4;
            sf_footer* footer_ptr = (sf_footer*)((char*)header_ptr+totalsize-(SF_FOOTER_SIZE/8));
            footer_ptr->requested_size = size;
            footer_ptr->block_size = (header_block_size-sizediff)>>4;
            footer_ptr->allocated = 1;
            if (padding != 0) {
                header_ptr->header.padded = 1;
                footer_ptr->padded = 1;
            } else {
                header_ptr->header.padded = 0;
                footer_ptr->padded = 0;
            }
         }
        //return pointer to original block
        return ptr;
    }
	return NULL;
}

void sf_free(void *ptr) {
    /* Mark an allocated block as free. Set fields and add to free list.
    If ptr is invalid, call abort(). */
    // check if ptr is valid
    if (check_invalid_pointer(ptr)) {   // 1 if invalid - abort
        abort();
    }
    sf_free_header* header_ptr = (sf_free_header*)((char*)ptr-(SF_HEADER_SIZE/8));
    // determine if next block can be coalesced.
    sf_free_header* next_block = (sf_free_header*)((char*)header_ptr+(header_ptr->header.block_size<<4));
    if (next_block->header.allocated == 0) {
        coalesce_fwd(header_ptr, next_block);  // combine blocks; this will just set header and footer blocksize
    }
    //set allocated bits
    sf_footer* footer_ptr = (sf_footer*)((char*)header_ptr+(header_ptr->header.block_size<<4)-(SF_FOOTER_SIZE/8));
    header_ptr->header.allocated = 0;
    footer_ptr->allocated = 0;
    // insert to free list
    insert_to_freelist(header_ptr);
	return;
}