#include <algorithm>
#include <cinttypes>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#define PAGE_BITMASK ((1 << OFFSET_WIDTH) - 1)

typedef struct swappedOutPageStruct {
    uint64_t virtualAddress;
    word_t frameIndex;
    uint64_t fatherPhysicalAddress ,cyclicDistance ;
} swappedOutPageStruct;


/**
 * Extract the offset of the virtual memory, depend on the level size.
 * @param virtualAddress uint64_t virtual address
 * @param level size_t level on the memory hierarchy
 * @return The offset inside level
 */
uint64_t extractOffsetInsideLevel(uint64_t virtualAddress, size_t level) {
    return ((virtualAddress >> (OFFSET_WIDTH * (TABLES_DEPTH + 1) - ((level + 1) * OFFSET_WIDTH))) & PAGE_BITMASK);
}

/**
 * Fill frame with zeros.
 * @param frameIndex word_t frame to fill
 */
void clearFrame(word_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i)
        PMwrite((frameIndex * PAGE_SIZE) + i, 0);
}

/*
 * Initialize the virtual memory.
 */
void VMinitialize() {
    clearFrame(0);
}

/**
 * Update the max cyclic distance with the relevant page.
 * @param frameIndex word_t index of the frame
 * @param pageSwappedIn uint64_t
 * @param leafVirtualAddress uint64_t virtual address of the leaf
 * @param swappedOutPage swapped out page struct
 * @param fatherPhysicalAddress uint64_t physical address of the father
 */
word_t updateMaxCyclicDistance(word_t frameIndex, uint64_t pageSwappedIn, uint64_t leafVirtualAddress,
                               swappedOutPageStruct &swappedOutPage, uint64_t fatherPhysicalAddress) {
    uint64_t absDifference = pageSwappedIn > leafVirtualAddress ? pageSwappedIn - leafVirtualAddress :
            leafVirtualAddress - pageSwappedIn;
    uint64_t cyclicDistance = std::min<uint64_t>(NUM_PAGES - absDifference, absDifference);
    swappedOutPage.frameIndex = swappedOutPage.cyclicDistance > cyclicDistance ? swappedOutPage.frameIndex : frameIndex;
    swappedOutPage.virtualAddress = swappedOutPage.cyclicDistance > cyclicDistance ?
            swappedOutPage.virtualAddress : leafVirtualAddress;
    swappedOutPage.fatherPhysicalAddress = swappedOutPage.cyclicDistance > cyclicDistance ?
            swappedOutPage.fatherPhysicalAddress : fatherPhysicalAddress;
    swappedOutPage.cyclicDistance = std::max(cyclicDistance, swappedOutPage.cyclicDistance);
    return 0;
}

/**
 * DFS Running on virtual memory tree
 * @param callingFrameIndex word_t current calling frame index
 * @param frameIndex word_t current frame index
 * @param maxIndex word_t max index
 * @param level size_t current level
 * @param pageSwappedIn uint64_t page swapped in
 * @param currentVirtualPath uint64_t current virtual path
 * @param swappedOutPage swapped out page struct
 * @param fatherPhysicalAddress uint64_t physical address of the father index
 * @return frame index if it is empty frame, 0 otherwise
 */
word_t dfs(word_t callingFrameIndex, word_t frameIndex, word_t &maxIndex, size_t level,
        uint64_t pageSwappedIn, uint64_t currentVirtualPath, swappedOutPageStruct &swappedOutPage,
        uint64_t fatherPhysicalAddress) {
    if (level >= TABLES_DEPTH)
        return updateMaxCyclicDistance(frameIndex, pageSwappedIn, currentVirtualPath, swappedOutPage,
                                       fatherPhysicalAddress);

    word_t childIndex = 0;
    bool allZeros = true;
    for (size_t j = 0; j < PAGE_SIZE; ++j) {
        uint64_t currentPhysicalAddress = (frameIndex * PAGE_SIZE) + j;
        PMread(currentPhysicalAddress, &childIndex);
        maxIndex = std::max(maxIndex, childIndex);
        if (childIndex != 0) {
            allZeros = false;
            uint64_t nextVirtualPath = (currentVirtualPath << OFFSET_WIDTH) + j;
            word_t emptyFrameIndex = dfs(callingFrameIndex, childIndex, maxIndex, level + 1, pageSwappedIn,
                                      nextVirtualPath, swappedOutPage, currentPhysicalAddress);
            if (emptyFrameIndex != 0) {
                //unlink to empty frame
                if (emptyFrameIndex == childIndex)
                    PMwrite(currentPhysicalAddress, 0);
                return emptyFrameIndex;
            }
        }
    }
    if (allZeros && frameIndex != callingFrameIndex) return frameIndex;
    return 0;

}

/**
 * Initialize DFS parameters and 
 * @param virtualAddress
 * @param callingFrameIndex
 * @param maxIndex
 * @param swappedOutPage
 * @return
 */
word_t runDfs(uint64_t virtualAddress, word_t callingFrameIndex, word_t &maxIndex,
              swappedOutPageStruct &swappedOutPage) {
    word_t initializeIndex = 0;
    size_t dfsLevel = 0;
    uint64_t pageSwappedIn = virtualAddress >> OFFSET_WIDTH, currentVirtualPath = 0, fatherPhysicalAddress = 0;
    return dfs(callingFrameIndex, initializeIndex, maxIndex, dfsLevel, pageSwappedIn, currentVirtualPath,
               swappedOutPage, fatherPhysicalAddress);
}

/**(level + 1 < )
 * This functions handle the page fault case of pages swapping.
 * @param address uint64_t address to write to
 * @param swappedOutPage swapped Out Page Struct
 * @return the frame index of the swapping page
 */
word_t swapPagesCase(uint64_t address, const swappedOutPageStruct &swappedOutPage, size_t level) {
    PMevict(swappedOutPage.frameIndex, swappedOutPage.virtualAddress);
    if (level + 1 < TABLES_DEPTH)
        clearFrame(swappedOutPage.frameIndex);
    PMwrite(swappedOutPage.fatherPhysicalAddress, 0);
    PMwrite(address, swappedOutPage.frameIndex); //new link
    return swappedOutPage.frameIndex;
}

/**
 * This functions handle the page fault case of max frame index < NUM_FRAMES.
 * @param address uint64_t address to write to
 * @param maxFrameIndex word_t max frame index
 * @return word_t maxFrameIndex
 */
word_t maxFrameIndexCase(uint64_t address, word_t maxFrameIndex, size_t level) {
    if (level + 1 < TABLES_DEPTH)
        clearFrame(maxFrameIndex);
    PMwrite(address, maxFrameIndex);
    return maxFrameIndex;
}

/**
 * This functions handle the page fault case of empty frame (frame fill with zeros).
 * @param address uint64_t address to write to
 * @param emptyFrame word_t empty frame index
 * @return empty frame index
 */
word_t emptyFrameCase(uint64_t address, word_t emptyFrame) {
    PMwrite(address, emptyFrame);
    return emptyFrame;
}

/**
 * This functions handle page fault creating while trying to access frame with index of zero.
 * @param virtualAddress int64_t of the virtual address
 * @param callingFrameIndex word_t of the calling frame index
 * @param address uint64_t address of the calling frame
 * @param level size_t depth level on the hierarchy virtual memory tree
 * @return word_t of the new frame to point to. Creating according to relevant scenario
 */
word_t pageFault(uint64_t virtualAddress, word_t callingFrameIndex, uint64_t address, size_t level) {
    word_t newFrameIndex, maxFrameIndex = 0;
    swappedOutPageStruct swappedOutPage;
    swappedOutPage.virtualAddress = 0;
    swappedOutPage.frameIndex = 0;
    swappedOutPage.fatherPhysicalAddress = 0;
    swappedOutPage.cyclicDistance = 0;
    word_t emptyFrame = runDfs(virtualAddress, callingFrameIndex, maxFrameIndex, swappedOutPage);
    if (emptyFrame != 0)
        newFrameIndex = emptyFrameCase(address, emptyFrame);
    else if (++maxFrameIndex < NUM_FRAMES)
        newFrameIndex = maxFrameIndexCase(address, maxFrameIndex, level);
    else newFrameIndex = swapPagesCase(address, swappedOutPage, level);
    if (level + 1 == TABLES_DEPTH)
        PMrestore(newFrameIndex, virtualAddress >> OFFSET_WIDTH);
    return newFrameIndex;
}


/**
 * Convert Virtual address to physical address by finding/creating path to the relevant page.
 * @param virtualAddress uint64_t of the virtual address
 * @param level size_t depth level on the hierarchy virtual memory tree
 * @param frameIndex word_t index on the relevant frame
 * @return uint64_t of the physical address
 */
uint64_t findPhysicalAddress(uint64_t virtualAddress, size_t level = 0, word_t frameIndex = 0) {
    uint64_t offsetInsideLevel = extractOffsetInsideLevel(virtualAddress, level);
    uint64_t currentPhysicalAddress = (frameIndex * PAGE_SIZE) + offsetInsideLevel;
    if (level >= TABLES_DEPTH)
        return currentPhysicalAddress;
    word_t addressContent = 0;
    PMread(currentPhysicalAddress, &addressContent);
    if (addressContent == 0)
        addressContent = pageFault(virtualAddress, frameIndex, currentPhysicalAddress, level);
    return findPhysicalAddress(virtualAddress,level + 1, addressContent);

}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t *value) {
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) return 0;
    PMread(findPhysicalAddress(virtualAddress), value);
    return 1;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value) {
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) return 0;
    PMwrite(findPhysicalAddress(virtualAddress), value);
    return 1;
}