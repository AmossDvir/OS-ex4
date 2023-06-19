//
// Created by user on 6/12/2023.
//

#include <iostream>
#include <bitset>
#include "VirtualMemory.h"

#define SUCCESS 1
#define FAILURE 0

/**
 * Extracts the page number from the virtual address
 * @param virtualAddress
 * @return
 */
uint64_t extractPageNumber (uint64_t virtualAddress)
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
uint64_t extractOffset (uint64_t virtualAddress)
{
  const uint64_t offsetMask = (1ULL << OFFSET_WIDTH) - 1;
  return virtualAddress & offsetMask;
}

void fillFrameWithZeros (uint64_t frameNumber)
{
  for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
      PMwrite (PAGE_SIZE * frameNumber + i, 0);
    }
}

/**
 * Validates the page number
 * @param pageNum
 * @return
 */
bool validatePageNumber (uint64_t pageNum)
{
  return (pageNum < VIRTUAL_MEMORY_SIZE);
}

uint64_t concatenateBits (uint64_t frameNum, uint64_t offset)
{
  return (frameNum << OFFSET_WIDTH) | offset;
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




uint64_t findUnusedFrame(word_t table,uint64_t frameNum){

  for(int i=0;i<TABLES_DEPTH;i++){
      word_t entry;
      PMread (table+i,&entry);
      if(entry!=0){
          //
        }
    }
  return -1;

}


//uint64_t cyclicNum(){
// // min{NUM_PAGES - |page_swapped_in - p|,
//  //|page_swapped_in - p|} is maximal
//
//
//}

/**
 * DFS
 */
uint64_t
traverseThroughTable (uint64_t page, uint64_t offset, int treeLevel, uint64_t baseAddress, uint64_t physicalAddress)
{
  //    Base cond:
  if (treeLevel == TABLES_DEPTH - 1)
    {
      uint64_t valueAddress = concatenateBits (physicalAddress, offset);
      return valueAddress;
    }

  uint64_t lastBits = (1ULL << (treeLevel * NUM_PAGES)) & page;
//  int shiftAmount = (TABLES_DEPTH - treeLevel - 1) * NUM_PAGES;
//  uint64_t currentPage = (page >> shiftAmount) & ((1ULL << NUM_PAGES) - 1);
  baseAddress = baseAddress + lastBits;

  word_t newAddr;
  PMread (baseAddress, &newAddr);

  if (baseAddress == 0)
    {
      //todo: Find an empty frame/evict
      //Creates new frame:
      uint64_t frameNum= findUnusedFrame (0,0);
      if(frameNum==-1){
          PMevict (0,0);
          //delete parents
      }
      fillFrameWithZeros (newAddr);
//      PMwrite ()
    }

  return traverseThroughTable (page, offset,
                               treeLevel + 1, baseAddress, newAddr);
}


uint64_t
iterations (uint64_t virtualAddress, uint64_t baseAddress,uint64_t frameIndex)
{
    uint64_t offset=virtualAddress%PAGE_SIZE;
    uint64_t pages=virtualAddress/PAGE_SIZE;
    uint64_t framesInUse [TABLES_DEPTH];
//    uint64_t numBitsPage=(VIRTUAL_ADDRESS_WIDTH-OFFSET_WIDTH)/TABLES_DEPTH;

    for(int i=0;i<TABLES_DEPTH;i++){
        uint64_t currentBits =(pages >> (OFFSET_WIDTH * (TABLES_DEPTH - i - 1))) & ((1 << OFFSET_WIDTH) - 1);
        uint64_t newAddress=frameIndex*PAGE_SIZE+currentBits;
        word_t f;
        PMread(newAddress,&f);
        if(f==0){
            //case 1: a frame containing an empty table - all rows are 0, remove reference from its parents


            //case 2: an unused frame, keep variable with maximal frame index reference from any table we visit, if
            // max_frame_index+1 < NUM_FRAMES then we know that the frame in the index (max_frame_index + 1) is unused.


            //case 3: ll frames are already used. swapped + cyclic
        }
    }

//        uint64_t frameNum= findUnusedFrame (0,0);
//        if(frameNum==-1){
//            PMevict (0,0);
//            //delete parents
//        }
//        fillFrameWithZeros (newAddr);
//      PMwrite ()
    return 0;
}


//uint64_t calculateLog2(uint64_t n) {
//    uint64_t logValue = 0;
//    while (n > 1) {
//        n >>= 1;
//        logValue++;
//    }
//    return logValue;
//}

//void iterateAllTablePages(uint64_t tablesPages,uint64_t offset){
//    uint64_t pagesLeft=tablesPages;
//    uint64_t pageBitsNum=calculateLog2(PAGE_SIZE);
//    int numOfTablesPages=2;
//    for(int i=0;i<numOfTablesPages;i++){
//        uint64_t currentPage = pagesLeft & ((1LL << pageBitsNum) - 1);
//        pagesLeft >>= pageBitsNum;
////        uint64_t physicalAddress=0;///
////        uint64_t frameFound=traverseThroughTable(currentPage,offset,0,0,physicalAddress);
//    }
//}


/**
 * Translates virtual address to a physical address
 * @param virtualAddress
 * @param physical_address
 * @return
 */
int
translateLogicToPhysical (uint64_t virtualAddress, uint32_t *physical_address)
{
  // [011101001][010]
  uint64_t offset = extractOffset (virtualAddress);
  uint64_t pageNum = extractPageNumber (virtualAddress); // p1[011]p2[101]p3[001]
  if (!validatePageNumber (pageNum))
    {
      return FAILURE;
    }
//  traverseThroughTable (pageNum, offset, 0, 0,);

  //    *physical_address = (frameNum << OFFSET_WIDTH) | offset;
  return SUCCESS;
}

/*
 * Initialize the virtual memory.
 */
void VMinitialize ()
{
  PMwrite (0, 0);
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread (uint64_t virtualAddress, word_t *value)
{
  uint32_t physical_address;
  int ret = translateLogicToPhysical (virtualAddress, &physical_address);

  // If the translation failed, then do nothing.
  if (ret == FAILURE)
    {
      return FAILURE;
    }

  // Read the value from the physical address.
//  traverseThroughTable
  PMread (physical_address, value);
  return SUCCESS;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite (uint64_t virtualAddress, word_t value)
{
  uint32_t physical_address;
  int ret = translateLogicToPhysical (virtualAddress, &physical_address);
  if (ret == FAILURE)
    {
      return FAILURE;
    }
//    traverseThroughTable
  PMwrite (physical_address, value);
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

int main(){
    uint64_t num=0b11110101000101100001;;
    iterations(num,0,0);
    return 0;
}