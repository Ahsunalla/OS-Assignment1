/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include "mm_aux.c"
#include <stdint.h>
#include "mm.h"


/* Proposed data structure elements */

typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block 
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (BlockHeader *)((uintptr_t)(p->next) & ~1)
#define SET_NEXT(p,n)  p->next = (BlockHeader *)((uintptr_t)n | ((uintptr_t)p->next & 1))
#define GET_FREE(p)    (uint8_t) ( (uintptr_t) (p->next) & 0x1 )   /* OK -- do not change */
#define SET_FREE(p,f)  p->next = (BlockHeader *)(((uintptr_t)p->next & ~1) | (f & 1))
#define SIZE(p)  (size_t)((uintptr_t)GET_NEXT(p) - (uintptr_t)(p + 1))

#define MIN_SIZE     (8)   // A block should have at least 8 bytes available for the user


static BlockHeader * first = NULL;
static BlockHeader * current = NULL;



/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
    // Align memory start and end (round memory_start up and memory_end down)
    uintptr_t aligned_memory_start = (memory_start + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
    uintptr_t aligned_memory_end = memory_end & ~(sizeof(void*) - 1);

    BlockHeader* last;

    // Check if already initialized
    if (first == NULL) {
    // Ensure there is enough room for at least one free block and the last block
    if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
        // Place the first block at the aligned start of the memory
        first = (BlockHeader*)aligned_memory_start;
        SET_FREE(first, 1); // Mark the first block as free

        // Place the last block at the end of the memory
        last = (BlockHeader*)((uintptr_t)first + SIZE(first) + sizeof(BlockHeader));
        SET_FREE(last, 0); // Last block is not free

        // Link first block to the last block
        SET_NEXT(first, last);

        // Set the current pointer to the first free block
        current = first;
    }
}
}

/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation. 
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */
void* simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }

    // Align the requested size to a multiple of the system's pointer size (e.g., 4 or 8 bytes)
    size_t aligned_size = (size + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);

    // Search for a free block
    BlockHeader* search_start = current;
    do {
        if (GET_FREE(current)) {
            // Coalesce consecutive free blocks (optional, can be added later)

            // Check if the free block is large enough
            if (SIZE(current) >= aligned_size) {
                // Check if the remainder is large enough for a new block
                if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    // Use the block as is and mark it non-free
                    SET_FREE(current, 0);
                } else {
                    // Carve out the block and leave the remainder as a new free block
                    uintptr_t new_block_address = (uintptr_t)current + aligned_size + sizeof(BlockHeader);
                    BlockHeader* new_block = (BlockHeader*)new_block_address;
                    
                    // Link the new block to the next block
                    SET_NEXT(new_block, GET_NEXT(current));
                    SET_FREE(new_block, 1);  // Mark the new block as free
                    
                    // Update the current block's link to the new block
                    SET_NEXT(current, new_block);
                    SET_FREE(current, 0);  // Mark the current block as allocated
                }
                
                // Return the address of the block's usable memory (after the header)
                return (void *)(current + 1);  // Return the address of the allocated memory
            }
        }

        current = GET_NEXT(current);  // Move to the next block
    } while (current != search_start);

    // No suitable block found
    return NULL;
}

/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation. 
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
void simple_free(void* ptr) {
    if (ptr == NULL) {
        return; // Nothing to free
    }

    // Get the block corresponding to the given pointer
    BlockHeader* block = (BlockHeader*)ptr - 1;

    // Check if the block is already free
    if (GET_FREE(block)) {
        return; // Block is already free, so this is probably an error
    }

    // Mark the block as free
    SET_FREE(block, 1);

    // Coalesce consecutive free blocks
    BlockHeader* next_block = GET_NEXT(block);
    if (next_block != NULL && GET_FREE(next_block)) {
        // Coalesce with the next block
        // Coalesce with the next block
        block->next = (BlockHeader *)((uintptr_t)block->next + sizeof(BlockHeader) + SIZE(next_block));
        SET_NEXT(block, GET_NEXT(next_block));
    }

    // Optionally, reset current to the block being freed
    current = block;
}

