//
// Created by user on 6/12/2023.
//

#include <iostream>
#include "VirtualMemory.h"

#define SUCCESS 1
#define FAILURE 0

const uint64_t offsetMask = (1ULL << OFFSET_WIDTH) - 1;
const uint64_t pageMask = ~offsetMask;

/**
 * Extracts the page number from the virtual address
 * @param virtualAddress
 * @return
 */
uint64_t extractPageNumber(uint64_t virtualAddress)
{
    return virtualAddress & pageMask;
}

/**
 * Extracts the offset from the virtual address
 * @param virtualAddress
 * @return
 */
uint64_t extractOffset(uint64_t virtualAddress)
{
    return virtualAddress & offsetMask;
}


void fillFrameWithZeros(uint64_t frameNumber)
{
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
        PMwrite(PAGE_SIZE * frameNumber + i, 0);
    }
}


/**
 * Validates the page number
 * @param pageNum
 * @return
 */
bool validatePageNumber(uint64_t pageNum)
{
    return (pageNum < VIRTUAL_MEMORY_SIZE);
}



void newFunction (word_t value)
{
    //iterate on every level of the tree - if it is the last one we reached
    // a leaf
    for (int level = 0; level < TABLES_DEPTH; level++)
    {
        //if this is the last layer
        if (level == TABLES_DEPTH)
        {
            uint64_t parseAddressLast = 0;//todo
            PMwrite (parseAddressLast, value);
            break;
        }

        uint64_t tempAddress;
        word_t valueAddress;
        uint64_t parseAddress = 0;//todo: for example 101
        uint64_t rootAddress = 0 * PAGE_SIZE;
        uint64_t currentAddress = rootAddress + parseAddress;
        PMread (currentAddress, &valueAddress);
        if (valueAddress == 0)
        {
            /* find unused frame or evict*/
            word_t tempFrame = 0;//f1 todo check the type
            if (level < TABLES_DEPTH - 1)
            {//todo check what the fit condition - do it
                // only if the next layer is a table
                fillFrameWithZeros (valueAddress);
            }
            PMwrite (currentAddress, tempFrame);
            tempAddress = tempFrame;
        }
    }

}



/**
 *
 */                                                      //      000000000000   1ULL << (treeLevel * NUM_PAGES)
void traverseThroughTable(uint64_t page, int treeLevel, uint64_t baseAddress) //dfs  100110011001  [100][110][011][001]
{
//    Base cond:
  if (treeLevel >= TABLES_DEPTH)
    {
      return;
    }

  uint64_t lastBits = (1ULL << (treeLevel * NUM_PAGES)) & page;
  baseAddress = baseAddress + lastBits;
  if (baseAddress == 0)
    {
      //Creates new frame:

    }

  word_t newAddr;
  PMread (baseAddress, &newAddr);

//  traverseThroughTable (page, treeLevel + 1, baseAddress)

}




/**
 * Translates virtual address to a physical address
 * @param virtualAddress
 * @param physical_address
 * @return
 */
int translateLogicToPhysical(uint64_t virtualAddress, uint32_t *physical_address)
{
    // [011101001][010]
    uint64_t offset = extractOffset(virtualAddress);
    uint64_t pageNum = extractPageNumber(virtualAddress); // p1[011]p2[101]p3[001]
    if (!validatePageNumber(pageNum))
    {
        return FAILURE;
    }
    traverseThroughTable(pageNum, 0, 0, );
//    *physical_address = (frameNum << OFFSET_WIDTH) | offset;
    return SUCCESS;
}


/*
 * Initialize the virtual memory.
 */
void VMinitialize()
{
    PMwrite(0, 0);
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t *value)
{
    uint32_t physical_address;
    int ret = translateLogicToPhysical(virtualAddress, &physical_address);

    // If the translation failed, then do nothing.
    if (ret == FAILURE)
    {
        return FAILURE;
    }

    // Read the value from the physical address.
    PMread(physical_address, value);
    return SUCCESS;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value)
{
    uint32_t physical_address;
    int ret = translateLogicToPhysical(virtualAddress, &physical_address);
    if (ret == FAILURE)
    {
        return FAILURE;
    }

    PMwrite(physical_address, value);
    return SUCCESS;
}




/*** paging:
 * divide the virtual memory into pages
 * divide the physical memory into frames (page and frame are in same size)
 * save mapping of each page belongs to each frame in physical memory
 * the operating system will save list of available frames in the physical
    memory and when a new process with n pages come, we will give him n
    available frames from the physical memory
 * build page table that represent where each page is in the physical memory
***/

/***
mmu -> divide to page number & offset (p|d)
    ->go to pages table (p the line in number of page)
    -> f frame number
    ->convert to a new (f|d) frame and same offset that is new address
    ->go to physical address

    * valid bit if the page really in the physical memory
    * modified(dirty bit)-whether the page was modifies since the last time we
      brought it to the memory

      if there are no empty frame - we will move a page to the hard disc if
      it was modified, otherwise deletes it and move on
    *when was the last access to page-timestamp
    * whan a page we want to access not mapped to a frame(valid bit =0)
    * there is page fault
*/