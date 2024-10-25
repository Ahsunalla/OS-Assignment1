/**
 * @file   check_mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Unit tests and suite for the memory management sub system.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <check.h>
#include "mm.h"

/* Choose which malloc/free to test */
#define MALLOC simple_malloc
#define FREE   simple_free

/**
 * @name: Utility function to XOR a block of memory. 
 */
uint32_t sum_block(uint32_t *data, uint32_t size)
{
  uint32_t sum = 0;
  uint32_t n;
  for (n=0; n < (size) >> 2; n++) {
    sum ^= data[n];
  }
  return sum;
}

/**
 * @name   Example simple allocation unit test
 * @brief  Tests whether simple allocation works.
 */
START_TEST (test_simple_allocation)
{
  int *ptr1;

  ptr1 = MALLOC(10 * sizeof(int));

/* Test whether each pointer have unique addresses*/
  ck_assert(ptr1 != 0);

  FREE(ptr1);
}
END_TEST

/**
 * @name   Example allocation overlap unit test.
 * @brief  Tests whether two allocations overlap.
 */
START_TEST (test_simple_unique_addresses)
{
  int *ptr1;
  int *ptr2;

  ptr1 = MALLOC(10 * sizeof(int));
  ptr2 = MALLOC(10 * sizeof(int));

/* Test whether each pointer have unique addresses*/
  ck_assert(ptr1 + 10 <= ptr2 || ptr2 + 10 <= ptr1);

  FREE(ptr1);
  FREE(ptr2);
}
END_TEST

/* Print debug messages to show what the test is doing. */
#define VERBOSE_OUTPUT 0

/**
 * @name   Memory exerciser
 * @brief  Allocate and use memory blocks of varying sizes.
 */
START_TEST (test_memory_exerciser)
{
  uint32_t iterations = 1000;                     /* Alter as required */

/* Struct to keep track of allocations */
  struct
  {
    void *addr;                                   /* Pointer returned by alloc */
    uint32_t *data;
/* Pointer used for accessing region (will differ
             from "addr" if alignment is handled by test) */
    uint32_t size;                                /* Size of requested block */
    uint32_t crc;                                 /* Checksum of contents of block */
  } blocks[16];

  uint32_t clock;
  uint32_t total_memory_size=0;
  uint32_t n;

  for(clock=0; clock<16; clock++) {
    blocks[clock].addr=0;
  }

  clock=0;

  while(iterations--) {
    char *addr;

/* randomize the size of a block. */
    blocks[clock].size=(24*1024*1024-total_memory_size)*(rand()&(1024*1024-1))/
      (1024*8);

/* Sanity check the block size. */
    if ((blocks[clock].size>0) && (blocks[clock].size<(24*1024*1024))) {

/* Try to allocate memory. */
      addr = MALLOC(blocks[clock].size);

/* Check if it was successful. */
      ck_assert_msg(addr != NULL, "Memory allocation failed!");

/* Verify that address is 8 byte aligned */
      if ((uintptr_t) addr & 0x07) {
        printf("Unaligned address %p returned!\n", addr);
        ck_assert(0);
      }

      blocks[clock].data = (uint32_t *) addr;

#if VERBOSE_OUTPUT
      printf("alloc[%02d] %d bytes, total=%d\n", clock, blocks[clock].size, total_memory_size);
#endif

/* Fill memory with data for verification */
      {
        uint32_t sum = 0;
        uint32_t x;
        for (n=0; n < (blocks[clock].size) >> 2; n++) {
          x = (uint32_t) rand();
          blocks[clock].data[n] = x;
          sum ^= x;
        }
        blocks[clock].crc = sum;
      }

/* Keep track of how much memory we have allocated... */
      total_memory_size+=blocks[clock].size;

/* and the address. */
      blocks[clock].addr=addr;
    }
    else {
      blocks[clock].addr=0;
    }

/* Move on to next block */
    clock=(clock+1)&15;

/* Verify all existing blocks before free */
    {
      int all_ok = 1;
      for (n=0; n < 16; n++) {
        if (blocks[n].addr != NULL) {
          uint32_t sum = sum_block(blocks[n].data, blocks[n].size);

          if (blocks[n].crc != sum) {
            printf("Checksum failed for block %d at addr=%p: %08x != %08x\n",
              n, blocks[n].addr, blocks[n].crc, sum);
            all_ok = 0;
          }
        }
      }
      ck_assert_msg(all_ok, "Pre-free memory block corruption found\n");
    }

/* Try to free one block. */
    if (0 != blocks[clock].addr) {
#if VERBOSE_OUTPUT
      printf("free [%02d] %d bytes\n", clock, blocks[clock].size);
#endif

      FREE(blocks[clock].addr);
      total_memory_size-=blocks[clock].size;

/* Mark block as free */
      blocks[clock].addr = NULL;

/* Verify all existing blocks after free */
      {
        int all_ok = 1;
        for (n=0; n < 16; n++) {
          if (blocks[n].addr != NULL) {
            uint32_t sum = sum_block(blocks[n].data, blocks[n].size);

            if (blocks[n].crc != sum) {
              printf("Checksum failed for block %d at addr=%p: %08x != %08x\n",
                n, blocks[n].addr, blocks[n].crc, sum);
              all_ok = 0;
            }
          }
        }
        ck_assert_msg(all_ok, "Post-free memory block corruption found\n");
      }
    }
  }

/* Free final blocks */
  for (clock=0; clock < 16; clock++) {
    if (blocks[clock].addr != NULL) {
#if VERBOSE_OUTPUT
      printf("free [%02d] %d bytes\n", clock, blocks[clock].size);
#endif
      uint32_t sum = sum_block(blocks[clock].data, blocks[clock].size);

      if (blocks[clock].crc != sum) {
        printf("Checksum failed for block %d: %08x != %08x\n", clock, blocks[clock].crc, sum);
        ck_assert(0);
      }
      FREE(blocks[clock].addr);
    }
  }
}
END_TEST

START_TEST (test_non_first_fit)
{
  // Allocate blocks of different sizes
  void *blockA = MALLOC(200);  // Block A (200 bytes)
  void *blockB = MALLOC(100);  // Block B (100 bytes)
  void *blockC = MALLOC(300);  // Block C (300 bytes)
  void *blockE = MALLOC(400);  // Block E (400 bytes)
  
  // Ensure allocations were successful
  ck_assert(blockA != NULL);
  ck_assert(blockB != NULL);
  ck_assert(blockC != NULL);
  ck_assert(blockE != NULL);

  // Free blocks A and C to create holes
  FREE(blockA);  // Creates a 200-byte hole
  FREE(blockC);  // Creates a 300-byte hole

  // Allocate a block D of size 180 bytes
  // First-fit would use blockA's space (200 bytes)
  // Best-fit would also use blockA's space (200 bytes is closer to 180 than 300)
  // So we need a different size to differentiate
  void *blockD = MALLOC(250);  // This size will differentiate between strategies

  // Ensure block D was successfully allocated
  ck_assert(blockD != NULL);

  if (blockD == blockA) {
    // If blockD is in blockA's space (first available hole), it's using first-fit
    ck_assert_msg(0, "Memory management appears to be using first-fit strategy");
  }
  // We don't abort if it's in blockC's space, as that's the expected best-fit behavior

  // Clean up remaining blocks
  FREE(blockB);
  FREE(blockD);
  FREE(blockE);
}
END_TEST

/**
 * @name   Example unit test suite.
 * @brief  Add your new unit tests to this suite.
 */
Suite* simple_malloc_suite()
{
  Suite *s = suite_create("simple_malloc");
  TCase *tc_core = tcase_create("Core tests");
  tcase_set_timeout(tc_core, 120);
  
  // new test("first_fit")
  tcase_add_test(tc_core, test_non_first_fit);

  // Existing tests
  tcase_add_test(tc_core, test_simple_allocation);
  tcase_add_test(tc_core, test_simple_unique_addresses);
  tcase_add_test(tc_core, test_memory_exerciser);

  suite_add_tcase(s, tc_core);
  return s;
}

/**
 * @name  Test runner
 * @bried This function runs the test suite and reports the result.
 */
int main()
{
  int number_failed;
  Suite *s = simple_malloc_suite();
  SRunner *sr = srunner_create(s);
  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? 0 : 1;
}
