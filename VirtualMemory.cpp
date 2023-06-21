//
// Created by user on 6/12/2023.
//

#include <iostream>
#include <bitset>
#include "VirtualMemory.h"

typedef struct
{
    uint64_t maxDistFrameIndex;
    uint64_t maxDistVal;
    uint64_t pageNumInLeaf;
    uint64_t pathToLeaf;
    uint64_t parent;
    uint64_t index;
} pageToEvictInfo;

/**
 * Extracts the page number from the virtual address
 * @param virtualAddress
 * @return
 */
uint64_t extractPageNumber(uint64_t virtualAddress)
{
    const uint64_t offsetMask = (1ULL << OFFSET_WIDTH) - 1;
    const uint64_t pageMask = ~offsetMask;
    return virtualAddress & pageMask;
}

/**
 * Extracts the offset from the virtual address
 * @param virtualAddress
 * @return
 */
uint64_t extractOffset(uint64_t virtualAddress)
{
    const uint64_t offsetMask = (1ULL << OFFSET_WIDTH) - 1;
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
bool validatePageNumber(uint64_t virtualAddress)
{
    return (virtualAddress < VIRTUAL_MEMORY_SIZE);
}

uint64_t concatenateBits(uint64_t frameNum, uint64_t offset)
{
    return (frameNum << OFFSET_WIDTH) | offset;
}

uint64_t cyclicNum(uint64_t pageSwappedIn, uint64_t p)
{
    auto cyclicDistance = abs(int(pageSwappedIn - p));
    return (NUM_PAGES - cyclicDistance) > cyclicDistance
           ? (uint64_t) cyclicDistance : (uint64_t) (NUM_PAGES - cyclicDistance);
}

bool isFrameEmptyTable(uint64_t frameIndex)
{
    word_t tmpValue;
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        PMread(frameIndex * PAGE_SIZE + i, &tmpValue);
        if (tmpValue != 0)
        {
            return false;
        }
    }
    return true;
}

uint64_t
case1(uint64_t pages, int treeLevel, uint64_t frameIndex, uint64_t parent, uint64_t callingFrameIndex, int childIndex = 0)
{

    if ((treeLevel == TABLES_DEPTH) || (frameIndex == callingFrameIndex))
    {
        return 0;
    }
    if (isFrameEmptyTable(frameIndex))
    {
        PMwrite(parent * PAGE_SIZE + childIndex, 0);
        return frameIndex;
    }
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        uint64_t cellInPM = frameIndex * PAGE_SIZE + i;
        word_t valInCell;
        PMread(cellInPM, &valInCell);
        if (valInCell != 0)
        {
            uint64_t childCheck = case1(pages, treeLevel + 1, valInCell, frameIndex, callingFrameIndex, i);
            if (childCheck != 0)
            {
                return childCheck;
            }
        }
    }
    return 0;
}

uint64_t case2(int treeLevel, uint64_t frameIndex)
{

    if (treeLevel == TABLES_DEPTH)
    {
        return frameIndex;
    }
    uint64_t maxFrameIndex = frameIndex;
    for (int i = 0; i < PAGE_SIZE; ++i)
    {
        uint64_t cellInPM = frameIndex * PAGE_SIZE + i;
        word_t valInCell;
        PMread(cellInPM, &valInCell);
        if (valInCell != 0)
        {
            uint64_t childCheck = case2(treeLevel + 1, valInCell);
            if (childCheck > maxFrameIndex)
            {
                maxFrameIndex = childCheck;
            }
        }
    }
    return maxFrameIndex;
}

pageToEvictInfo
case3(uint64_t pages, uint64_t treeLevel, uint64_t frameIndex, uint64_t
frameMaxCyclicDist, uint64_t maxCyclicDist, uint64_t parent, uint64_t index,
      uint64_t currentPage, pageToEvictInfo pageInfo)
{
    if (treeLevel == TABLES_DEPTH)
    {
        uint64_t cyclicVal = cyclicNum(currentPage, pages);
        return {frameIndex, cyclicVal, currentPage, 0, parent, index};
    } else
    {
        for (int i = 0; i < PAGE_SIZE; ++i)
        {
            uint64_t cellInPM = (frameIndex * PAGE_SIZE) + i;
            word_t valInCell;
            PMread(cellInPM, &valInCell);
            if (valInCell != 0)
            {
                uint64_t tmpPage = (currentPage << OFFSET_WIDTH) + i;
                pageInfo = case3(pages, treeLevel + 1, valInCell, frameMaxCyclicDist, maxCyclicDist, frameIndex, i, tmpPage, pageInfo);
                if (pageInfo.maxDistVal > maxCyclicDist)
                {
                    maxCyclicDist = pageInfo.maxDistVal;
                    frameMaxCyclicDist = pageInfo.maxDistFrameIndex;
                }
            }
        }
    }



    //page? return val?
    //path - shift left <<offsetwidth+i(child num)(shift left to temp address)
    return pageInfo;
}


//struct: frame ,max value, parent

uint64_t
translate(uint64_t virtualAddress, uint64_t
frameIndex)
{
    if (!validatePageNumber(virtualAddress))
    {
        return 0;
    }
    uint64_t offset = virtualAddress % PAGE_SIZE;
    uint64_t pages = virtualAddress / PAGE_SIZE;//ignore garbage values
    uint64_t framesInUse[TABLES_DEPTH];
    //    uint64_t numBitsPage=(VIRTUAL_ADDRESS_WIDTH-OFFSET_WIDTH)/TABLES_DEPTH;
    uint64_t availableFrame;//initialize?
    uint64_t physicalAddress;

    for (int i = 0; i < TABLES_DEPTH; i++)
    {
        uint64_t currentBits = (pages >> (OFFSET_WIDTH * (TABLES_DEPTH - i - 1)))
                               & ((1 << OFFSET_WIDTH)
                                  - 1);//check for different values
        uint64_t newAddress = frameIndex * PAGE_SIZE + currentBits;
        word_t frameVal;
        PMread(newAddress, &frameVal);
        if (frameVal == 0)//frameVal!=0 handle
        {
            //case 1: a frame containing an empty table - all rows are 0, remove reference from its parents
            uint64_t frameToSent = 0;
            uint64_t case1Frame = case1(pages, 0, 0, 0, frameIndex);//todo
            // frameIndex or 0?
            // calling frame
            if (case1Frame != 0)
            {
                PMwrite(newAddress, (word_t) case1Frame);
                frameIndex = case1Frame;
                continue;
            }

            //case 2: an unused frame, keep variable with maximal frame index reference from any table we visit, if
            // max_frame_index+1 < NUM_FRAMES then we know that the frame in the index (max_frame_index + 1) is unused.
            uint64_t case2Frame = case2(0, 0);

            //            // Reached the last level of the tree
            if (case2Frame + 1 < NUM_FRAMES)
            {
                PMwrite(newAddress, (word_t) case2Frame + 1);
                fillFrameWithZeros(case2Frame + 1);
                frameIndex = case2Frame + 1;//?
                continue;
            }

            //case 3: all frames are already used. swapped + cyclic
            pageToEvictInfo pageInfo = {0, 0, 0, 0, 0, 0};
            pageToEvictInfo pageToEvict = case3(pages, 0, 0, 0, 0, 0, 0, 0, pageInfo);

            //if pageToEvict.frameMaxCyclicDist in range , if
            // maxCyclicDist!=0?
            PMevict(pageToEvict.maxDistFrameIndex, pageToEvict.pageNumInLeaf);
            if (i != TABLES_DEPTH - 1)
            {
                fillFrameWithZeros(pageToEvict.maxDistFrameIndex);
            }
            PMwrite((pageToEvict.parent * PAGE_SIZE) + pageToEvict.index, 0);//??
            //if depth nit table depth - if not in leaf - reset frame
            //parent remove reference to the evicted page

            PMwrite(newAddress, (word_t) (pageToEvict.maxDistFrameIndex));
            frameIndex = pageToEvict.maxDistFrameIndex;
        } else
        {
            frameIndex = frameVal;
        }
    }
    PMrestore(frameIndex, pages);
    physicalAddress = concatenateBits(frameIndex, offset);
    return physicalAddress;
}

/*
 * Initialize the virtual memory.
 */
void VMinitialize()
{
    fillFrameWithZeros(0);
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
    uint64_t physical_address = translate(virtualAddress, 0);
    int ret = int(physical_address);

    // If the translation failed, then do nothing.
    if (ret == 0)
    {
        return 0;
    }
    // Read the value from the physical address.
    PMread(physical_address, value);
    return 1;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value)
{
    uint64_t physical_address = translate(virtualAddress, 0);
    int ret = int(physical_address);

    // If the translation failed, then do nothing.
    if (ret == 0)
    {
        return 0;
    }

    // Read the value from the physical address.
    PMwrite(physical_address, value);
    return 1;
}

