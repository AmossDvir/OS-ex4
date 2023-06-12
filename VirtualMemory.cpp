//
// Created by user on 6/12/2023.
//

#include "VirtualMemory.h"
#define SUCCESS 1
#define FAILURE 0
/*
 * Initialize the virtual memory.
 */
void VMinitialize(){
    PMwrite(0,0);
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value){
    if (virtualAddress > || virtualAddress < ){// todo: figure out when to return error
    return FAILURE;
    }
    return SUCCESS;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value){
    if (virtualAddress > || virtualAddress < ){// todo: figure out when to return error
        return FAILURE;
    }
    return SUCCESS;
}