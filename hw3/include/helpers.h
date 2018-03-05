#ifndef HELPERS_H
#define HELPERS_H

#include "sfmm.h"
#include <string.h>
#include <stdio.h>

int get_min_freelist(size_t blocksize);
sf_free_header* search_free_lists(int i, size_t totalsize, size_t req_size);
void remove_block(sf_free_header* node);
sf_free_header* split_block(sf_free_header* current, size_t alloc_blocksize, size_t sizediff);
void coalesce_back(sf_free_header* block_header);
void insert_to_freelist(sf_free_header* block_header);

int check_invalid_pointer(void* ptr);
void coalesce_fwd(sf_free_header* block_header, sf_free_header* next_block);

#endif
