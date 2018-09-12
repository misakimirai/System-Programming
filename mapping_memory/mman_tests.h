/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include <stdio.h>
#include <stdlib.h>

#define MAX_TEST_NUMBER 100

// type alias for test case function
typedef int testcase(void);

/**
 * Tests to see that syscall_mmap is able to allocate anonymous memory and play
 * nicely with the page fault handler during reads and writes. This
 * only requires syscall_mmap to be implemented properly.
 */
static testcase mmap_simple_test;

/**
 * Tests to see that syscall_mmap returns -1 on various types of failures, and
 * checks to see that illegal accesses result in segfaults.
 */
static testcase mmap_failure_test;

/**
 * Tests that syscall_mmap makes reasonably good use of the virtual address
 * space by performing multiple, large mappings and unmappings. It is not
 * particularly strict, and aims only to punish pathologically bad allocation
 * strategies.
 */
static testcase mmap_stress_allocation_test;

/**
 * Performs basic tests on syscall_sbrk. Also checks that VMAs from
 * syscall_mmap don't get allocated right on the program break.
 */
static testcase sbrk_test;

/**
 * Tests to see that private mappings to files are able to take advantage of
 * swapping out pages. In particular, when restricting main memory to a size
 * much smaller than swap, we are still able to fault in a file that is 5
 * times larger than the restricted main memory. This also tests that the
 * backing storage (i.e. file on disk) is not modified.
 */
static testcase private_file_swapout_test;

/**
 * Tests to see that writes performed on memory mapped with the MAP_SHARED
 * flag is visible across processes, with major faults occuring only
 * the first time a file page is brought from disk into the page cache. Also
 * tests that the backing storage (i.e. file on disk) is not updated until
 * `syscall_munmap` is called.
 *
 * This requires you to implement `load_shared_page` and `syscall_munmap`.
 */
static testcase load_shared_file_test;

/**
 * Tests to see, at a basic level, that all PTEs pointing to the same page that
 * maps the same file (from multiple processes and multiple VMAs in the same
 * process) have their dirty and present flags cleared right after
 * `load_shared_page` is called on that page.
 */
static testcase page_out_shared_file_test;

/**
 * The test layout is almost identical to `page_out_shared_file_test`.
 * However, it tests that all PTEs that point to the same file-backed page
 * are marked clean and present after a `syscall_msync` call, rather than
 * clean and not present. It also tests to see that backing storage is updated
 * after a call to `msync`.
 */
static testcase msync_test;

// End of testcase descriptions

/**
 * Prints usage information and exits in response to usage errors and parses a
 * test number otherwise.
 */
static int parse_test_number(int argc, char *argv[]);

/**
 * Maps test numbers to testcase function pointers.
 */
static testcase *get_testcase_pointer(unsigned test_number);

/**
 * Runs all tests as child processes to prevent tests from conflicting with
 * eachother.
 */
static int fork_and_run_all(char const *program_name);

/**
 * Recursively run the program using exec, but with a specific command line
 * argument.
 */
static int self_exec_one(char const *program_name, unsigned number);

static unsigned number_of_testcases = 0;

static testcase *all_testcases[MAX_TEST_NUMBER];

static char const *all_test_labels[MAX_TEST_NUMBER];

int main(int argc, char *argv[]);

#define SUCCESS 0
#define FAILURE 1

#define DECLARE_TESTCASE(number, testname)                                     \
                                                                               \
    static int testname(void);                                                 \
                                                                               \
    static int __##testname##_wrapper(void) {                                  \
        fprintf(stderr, "%u: Running %s!\n", (unsigned)(number), #testname);   \
        if (testname() == SUCCESS) {                                           \
            fprintf(stderr, "SUCCESS!\n");                                     \
            return SUCCESS;                                                    \
        }                                                                      \
        fprintf(stderr, "FAILURE!\n");                                         \
        return FAILURE;                                                        \
    }                                                                          \
                                                                               \
    __attribute__((constructor)) static void __init_##testname(void) {         \
        if (number > number_of_testcases) {                                    \
            number_of_testcases = number;                                      \
        }                                                                      \
        if (number_of_testcases > MAX_TEST_NUMBER) {                           \
            fprintf(stderr, "Limited to created no more than %u tests.\n",     \
                    MAX_TEST_NUMBER);                                          \
            exit(-1);                                                          \
        }                                                                      \
        all_testcases[number - 1] = __##testname##_wrapper;                    \
        all_test_labels[number - 1] = #testname;                               \
    }

#define testcase_assert(condition, msg, args...)                               \
    {                                                                          \
        if (!(condition)) {                                                    \
            fprintf(stderr, "%s:%u: ", __FILE__, __LINE__);                    \
            fprintf(stderr, msg, ##args);                                      \
            fprintf(stderr, "\n");                                             \
            return FAILURE;                                                    \
        }                                                                      \
    }
