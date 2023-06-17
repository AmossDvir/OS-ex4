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

/**
 * Validates the page number
 * @param pageNum
 * @return
 */
bool validatePageNumber(uint64_t pageNum)
{
    return (pageNum < VIRTUAL_MEMORY_SIZE);
}

/**
 * Translates virtual address to a physical address
 * @param virtualAddress
 * @param physical_address
 * @return
 */
int translateLogicToPhysical(uint64_t virtualAddress, uint32_t *physical_address)
{
    uint64_t offset = extractOffset(virtualAddress);
    uint64_t pageNum = extractPageNumber(virtualAddress);
    if (!validatePageNumber(pageNum))
    {
        return FAILURE;
    }

    //go to the page tables
    uint64_t frameNum = 0; //pages table[pageNum]
    *physical_address = (frameNum << OFFSET_WIDTH) | offset;
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


bool isPageInPhysical(uint64_t pageNum)
{
    return false;//page table [page][isValid]
}

void moveToHardDisk()
{
}

//layersNum=TABLES_DEPTH
void tryFunc(uint64_t virtualAddress, word_t value)
{
    uint64_t frameNum = 0;
    uint64_t tempOffset = 0;

    uint64_t offset = virtualAddress & offsetMask;//1
    uint64_t pageNum = virtualAddress & pageMask;//0110(6)

    //  uint64_t unusedFrame=0;
    uint64_t sum = 0;
    uint64_t physicalAddress = 0;
    for (int i = 0; i < TABLES_DEPTH; i++)
    {
        //      PMread(0 + 5, &addr1) // first translation
        //      If (addr1 == 0) then
        //      Find an unused frame or evict a page from some frame. Suppose this frame number is f1
        //      Write 0 in all of its contents (only necessary if next layer is a table)
        //      PMwrite(0 + 5, f1)
        //      addr1 = f1
        //      PMread(addr1 * PAGE_SIZE + 1, &addr2) // second translation
        //      If (addr2 == 0) then
        //      Find an unused frame or evict a page from some frame. Suppose this frame number is f2.
        //      Make sure you are not evicting f1 by accident
        //      Restore the page we are looking for to frame f2 (only necessary pointing to a page)
        //      PMwrite(addr1 * PAGE_SIZE + 1, f2)
        //      addr2 = f2
        //      PMwrite(addr2 * PAGE_SIZE + 6, value)
        //where to use offset?
        uint64_t currentBit = (pageNum >> i) & 1;
        sum += currentBit;
        word_t tempValue = 0;//
        //sholud check both options of the offset? (or even general number of
        // the offset)
        PMread(sum, &tempValue);
        if (tempValue != 0)
        {
            sum += 1;
            physicalAddress = (sum << OFFSET_WIDTH) | currentBit;
            PMwrite(physicalAddress, 0);
        }
    }
    PMrestore(sum, pageNum);//unused frame index?
    uint64_t addressToWriteTo = (sum << OFFSET_WIDTH) | offset;
    PMwrite(addressToWriteTo, value);
}


//go on the tree according to the levels
//according to the address move right and left on the tree
//if ==0

//PMread(0 + 5, &addr1) // first translation
//If (addr1 == 0) then
//    Find an unused frame or evict a page from some frame. Suppose this frame number is f1
//    Write 0 in all of its contents (only necessary if next layer is a table)
//PMwrite(0 + 5, f1)
//addr1 = f1
//PMread(addr1 * PAGE_SIZE + 1, &addr2) // second translation
//If (addr2 == 0) then
//    Find an unused frame or evict a page from some frame. Suppose this frame number is f2.
//Make sure you are not evicting f1 by accident
//Restore the page we are looking for to frame f2 (only necessary pointing to a page)
//PMwrite(addr1 * PAGE_SIZE + 1, f2)
//addr2 = f2
//PMwrite(addr2 * PAGE_SIZE + 6, value)




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