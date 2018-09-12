/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#include "disk.h"
#include "kernel.h"
#include "mman_tests.h"
#include "nommu.h"
#include "page_fault_helpers.h"
#include "physical_memory.h"
#include "syscalls.h"
#include "types.h"
#include "virtual_memory.h"
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    unsigned test_number = parse_test_number(argc, argv);
    if (test_number == 0) {
        exit(fork_and_run_all(argv[0]));
    } else {
        exit(get_testcase_pointer(test_number)());
    }
}

DECLARE_TESTCASE(1, mmap_simple_test);
DECLARE_TESTCASE(2, mmap_failure_test);
DECLARE_TESTCASE(3, mmap_stress_allocation_test);
DECLARE_TESTCASE(4, sbrk_test);
DECLARE_TESTCASE(5, private_file_swapout_test);
DECLARE_TESTCASE(6, load_shared_file_test);
DECLARE_TESTCASE(7, page_out_shared_file_test);
DECLARE_TESTCASE(7, msync_test);

/*****************************************************************************
 *                                                                           *
 *                 BEGIN TEST CASE DEFINITIONS                               *
 *                                                                           *
 ****************************************************************************/

#define MAP_PRIVANON MAP_ANONYMOUS | MAP_PRIVATE

static int mmap_simple_test(void) {
    context_switch(spawn_process());

    addr32 ptr =
        syscall_mmap(PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_PRIVANON, NULL, 0);
    testcase_assert(ptr != MAP_FAILED, "mmap returned failure.");
    char buffer[PAGE_SIZE];
    char empty_buffer[PAGE_SIZE] = {0};
    enum FaultResult res = 0;
    res = nommu_read_from_virtual_address(ptr, buffer, PAGE_SIZE);
    testcase_assert(res == FAULT_MINOR,
                    "read to unallocated page did not cause minor fault");
    testcase_assert(!memcmp(buffer, empty_buffer, PAGE_SIZE),
                    "fresh mmap memory not 0-initialized");

    char const *msg = "Let's see if this write operation works!";
    res = nommu_write_to_virtual_address(ptr, msg, strlen(msg));
    testcase_assert(res == FAULT_MINOR,
                    "write to clean page did not cause a fault");

    res = nommu_read_from_virtual_address(ptr, buffer, strlen(msg) + 20);
    testcase_assert(res == FAULT_NONE, "read from accessed page caused fault");
    testcase_assert(!strcmp(buffer, msg), "Read did not match written data");

    kill_process(current);
    return SUCCESS;
};

static int mmap_failure_test(void) {
    context_switch(spawn_process());

    addr32 ptr =
        syscall_mmap(PAGE_SIZE, PROT_NONE, MAP_SHARED | MAP_PRIVATE, NULL, 0);
    testcase_assert(ptr == MAP_FAILED, "mmap succeeded with illegal "
                                       "combination of MAP_SHARED and "
                                       "MAP_PRIVATE");

    ptr = syscall_mmap(PAGE_SIZE, PROT_NONE, MAP_PRIVANON, NULL, 0);
    testcase_assert(ptr != MAP_FAILED, "mmap failed");
    enum FaultResult res = 0;
    char buffer[256];
    res = nommu_read_from_virtual_address(ptr, buffer, 256);
    testcase_assert(
        res == FAULT_SEGV,
        "read from PROT_NONE memory did not cause segmentation fault");

    ptr = syscall_mmap(PAGE_SIZE, PROT_READ, MAP_PRIVANON, NULL, 0);
    testcase_assert(ptr != MAP_FAILED, "mmap failed");

    res = nommu_write_to_virtual_address(ptr, "hello", 5);
    testcase_assert(
        res == FAULT_SEGV,
        "write to PROT_READ memory did not cause segmentation fault");

    testcase_assert(syscall_munmap(ptr) == 0, "munmap failed");

    res = nommu_read_from_virtual_address(ptr, buffer, 256);
    testcase_assert(
        res == FAULT_SEGV,
        "read from unmapped memory did not cause segmentation fault");

    ptr = syscall_mmap(PAGE_SIZE, PROT_READ, MAP_SHARED, "nonexistent_file", 0);
    testcase_assert(ptr == MAP_FAILED,
                    "mmap succeeded despite being given nonexistent file");
    uint32_t too_big = current->stack_segment->end_addr -
                       current->heap_segment->end_addr + PAGE_SIZE;
    ptr = syscall_mmap(too_big, PROT_READ, MAP_PRIVANON, NULL, 0);
    testcase_assert(
        ptr == MAP_FAILED,
        "mmap succeeded despite being requested more space than between stack "
        "and heap");
    kill_process(current);
    return SUCCESS;
}

static int mmap_stress_allocation_test(void) {

    context_switch(spawn_process());

    addr32 stack = current->stack_segment->start_addr;
    addr32 brk = current->heap_segment->end_addr;
    uint32_t room = stack - brk;
    uint32_t sizes[5] = {room / 2, room / 8, room / 16, room / 32, room / 64};
    addr32 ptrs[5] = {0};
    for (unsigned i = 0; i < 5; ++i) {
        sizes[i] = page_align(sizes[i]); // page align each size
        ptrs[i] = syscall_mmap(sizes[i], PROT_READ, MAP_PRIVANON, NULL, 0);
        testcase_assert(ptrs[i] != MAP_FAILED, "mmap #%u failed", i)
            testcase_assert(
                ptrs[i] >= brk && ptrs[i] <= stack - sizes[i],
                "VMA #%u with boundaries (%#08x, %#08x) not between heap "
                "(ends at %#08x) and stack (starts at %#08x)!",
                i, ptrs[i], ptrs[i] + sizes[i], brk, stack);
        for (unsigned j = 0; j < i; ++j) {
            unsigned first = ptrs[i] < ptrs[j] ? i : j;
            unsigned second = first == i ? j : i;
            testcase_assert(ptrs[first] + sizes[first] <= ptrs[second],
                            "Overlap between VMA #%u with boundaries (%#08x, "
                            "%#08x) and VMA #%u "
                            "with boundaries (%#08x, %#08x)",
                            first, second, ptrs[first],
                            ptrs[first] + sizes[first], ptrs[second],
                            ptrs[second] + sizes[second]);
        }
    }
    for (unsigned i = 0; i < 4; ++i) {
        testcase_assert(syscall_munmap(ptrs[i]) == 0, "munmap #%u failed", i);
    }
    addr32 ptr =
        syscall_mmap(page_align(room / 4), PROT_READ, MAP_PRIVANON, NULL, 0);
    testcase_assert(ptr != MAP_FAILED, "mmap failed");
    testcase_assert(ptr >= brk && ptr + page_align(room / 4) <= stack,
                    "(%u, %u) crosses stack or heap", ptr, ptr + room / 2);
    kill_process(current);
    return SUCCESS;
}

static int sbrk_test(void) {

    context_switch(spawn_process());

    addr32 brk = syscall_sbrk(0);
    addr32 ptr = syscall_mmap(500 * PAGE_SIZE, PROT_WRITE | PROT_READ,
                              MAP_PRIVANON, NULL, 0);
    addr32 new_brk = syscall_sbrk(500 * PAGE_SIZE);

    // check to see that new program break is not failed and doesn't overlap
    // with the new VMA (if this fails, your MMAP implementation is bad)

    testcase_assert(new_brk != MAP_FAILED,
                    "unable to shift break (due to mmapped VMA placement?)");
    testcase_assert(new_brk < ptr, "break overlaps with mmap'd VMA");
    testcase_assert(brk + (500 << 12) == new_brk, "break shifted incorrectly");

    enum FaultResult res = FAULT_NONE;

    // check to see that a write of size 5, 3 bytes before the break, causes a
    // segfault

    res = nommu_write_to_virtual_address(new_brk - 3, "Segfault :)", 5);
    testcase_assert(res == FAULT_SEGV,
                    "Write past break did not cause segfault");

    // but check to see that the write of size 3 still worked

    char buff[PAGE_SIZE] = {0};
    res = nommu_read_from_virtual_address(new_brk - 3, buff, 3);
    testcase_assert(res == FAULT_NONE, "Got fault on recently written page");
    testcase_assert(!strcmp(buff, "Seg"),
                    "Expected to read \"Seg\", got \"%s\"", buff);

    kill_process(current);
    return SUCCESS;
}

static int private_file_swapout_test(void) {

    context_switch(spawn_process());

    // create a file full of size_t values in the range [0,  50 * PAGE_SIZE / 8)

    char filename[] = "./tmp/private_file_swapout";
    FILE *tmpfp = fopen(filename, "w+");
    testcase_assert(tmpfp, "Failed to open %s", filename);

    testcase_assert(!truncate(filename, 51 * PAGE_SIZE), "truncate failed");
    for (size_t i = 0; i < 51 * PAGE_SIZE / sizeof(i); ++i) {
        fwrite(&i, sizeof(i), 1, tmpfp);
    }
    addr32 ptr = syscall_mmap(50 * PAGE_SIZE, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE, filename, 0);

    // artificially set all but 50 pages to reserved to force swapping
    size_t pages_desired = 10;
    struct page *pg = NULL;
    addr32 accessible_pfns[10];
    while ((pg = get_next_unused_page(pg))) {
        if (pages_desired) {
            accessible_pfns[10 - pages_desired] =
                page_to_phys(pg) >> NUM_OFFSET_BITS;
            --pages_desired;
        } else {
            pg->reserved = 1;
        }
    }

    // BEGIN ACTUAL TESTING
    for (size_t i = 0; i < 50 * PAGE_SIZE / sizeof(i); ++i) {
        // major faults should occur at page boundaries
        bool should_fault = !misalignment(i * sizeof(size_t), NUM_OFFSET_BITS);
        size_t j = 0;
        enum FaultResult res = nommu_read(j, ptr, i);
        testcase_assert(res != FAULT_NOMEM, "Ran out of memory despite swap");
        testcase_assert(res == (should_fault ? FAULT_MAJOR : FAULT_NONE),
                        "#%zu: Expected %s fault", i,
                        should_fault ? "major" : "no");
        pte_t *pte = get_pte(current, ptr + sizeof(size_t) * i, false);
        testcase_assert(pte && pte->available,
                        "#%zu: PTE on virtual address not "
                        "available after resolved fault.",
                        i);

        // check that an inaccessible page was not somehow used
        bool is_accessible_pfn = false;
        for (size_t i = 0; i < 10 && !is_accessible_pfn; ++i) {
            is_accessible_pfn = pte->base_addr == accessible_pfns[i];
        }
        testcase_assert(is_accessible_pfn, "Data stored in illegal page frame");

        // check for correct read value
        testcase_assert(j == i, "#%zu: Expected %zu, got %zu", i, i, j);

        // Perform an increment on every mmapped integer so that
        // `nommu_read(var, ptr, i)` stores `i+1` in `var` instead of `i`
        ++j;
        res = nommu_write(j, ptr, i);
        testcase_assert(
            should_fault ? res == FAULT_MINOR : res == FAULT_NONE,
            "#%zu: Unexpected fault on write to %s page after recent read", i,
            should_fault ? "clean" : "dirty");
    }
    // file should not be modified, and reads should still give the right value
    fseek(tmpfp, 0, SEEK_SET);
    for (size_t i = 0; i < 50 * PAGE_SIZE / sizeof(i); ++i) {
        size_t j = 0;
        testcase_assert(fread(&j, sizeof(size_t), 1, tmpfp) == 1,
                        "fread failed");
        testcase_assert(j == i, "File was modified.");
        nommu_read(j, ptr, i);
        testcase_assert(j == i + 1, "Expected %zu, got %zu", i + 1, j);
    }

    size_t buff = 0;
    kill_process(current);
    fseek(tmpfp, 0, SEEK_CUR);
    for (size_t i = 0; i < 50 * PAGE_SIZE / sizeof(i); ++i) {
        buff = 0;
        testcase_assert(
            fwrite(&buff, sizeof(buff), 1, tmpfp) == 1,
            "#%zuth size_t in file was modified despite private mapping.\n", i);
    }
    fclose(tmpfp);
    unlink(filename);

    return SUCCESS;
}

static int load_shared_file_test(void) {
    char filename[] = "./tmp/load_shared_file";
    FILE *tmpfp = fopen(filename, "w+");
    testcase_assert(tmpfp, "Failed to open %s", filename);

    testcase_assert(!truncate(filename, 2 * PAGE_SIZE), "truncate failed");

    struct process *proc1 = spawn_process();
    struct process *proc2 = spawn_process();

    context_switch(proc1);

    // Process 1 maps entire file
    addr32 ptr1 = syscall_mmap(2 * PAGE_SIZE, PROT_WRITE | PROT_READ,
                               MAP_SHARED, filename, 0);
    testcase_assert(ptr1 != MAP_FAILED, "mmap failed");

    // If you didn't increment the mapped_file reference count in
    // syscall_mmap, you'll fail because of this line
    obtain_mapped_file("disk.h");

    // Process 2 maps file with offset of one page and writes "Header" to file
    context_switch(proc2);

    addr32 ptr2 = syscall_mmap(PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED,
                               filename, PAGE_SIZE);
    testcase_assert(ptr2 != MAP_FAILED, "mmap failed");

    testcase_assert(nommu_write("Header", ptr2, 0) == FAULT_MAJOR,
                    "Expected major fault on first write.");

    // Process 1 reads "Header" from the file
    context_switch(proc1);

    char buffer[PAGE_SIZE] = {0};
    testcase_assert(nommu_read(buffer, ptr1, 1) == FAULT_MINOR,
                    "Expected minor fault on first read.");

    testcase_assert(!strcmp(buffer, "Header"), "Did not read correct data.");

    // Backing file should be untouched still

    fseek(tmpfp, PAGE_SIZE, SEEK_SET);
    testcase_assert(fread(buffer, PAGE_SIZE, 1, tmpfp) == 1, "fread failed");
    testcase_assert(!memcmp(buffer, "\0\0\0\0\0\0\0", strlen("Header")),
                    "Physical backing file should be untouched before "
                    "munmap or msync");

    // Unmap process 2's VM reference to the file
    context_switch(proc2);
    testcase_assert(!syscall_munmap(ptr2), "munmap failed");
    context_switch(proc1);

    // Page count should ensure page was not removed from cache
    testcase_assert(nommu_read(buffer, ptr1, 1) == FAULT_NONE,
                    "Expected no fault on read to previously read page.");

    // Backing file should now be updated
    fseek(tmpfp, PAGE_SIZE, SEEK_SET);
    testcase_assert(fread(buffer, PAGE_SIZE, 1, tmpfp), "fread failed");
    testcase_assert(!strcmp(buffer, "Header"),
                    "Physical backing file should be updated after munmap.");

    // If we unmap ptr1, then the page should be evicted from the cache, since
    // its reference count is now 0
    testcase_assert(!syscall_munmap(ptr1), "munmap failed");

    // Therefore, trying to read the same page again will cause a major fault
    ptr1 = syscall_mmap(PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, filename,
                        PAGE_SIZE);

    testcase_assert(nommu_read(buffer, ptr1, 0) == FAULT_MAJOR,
                    "Expected major fault");
    kill_process(proc1);
    kill_process(proc2);

    fclose(tmpfp);
    unlink(filename);

    return SUCCESS;
}

static int page_out_shared_file_test(void) {
    char filename[] = "./tmp/page_out_shared_file";
    FILE *tmpfp = fopen(filename, "w+");
    testcase_assert(tmpfp, "Failed to open %s", filename);

    testcase_assert(!truncate(filename, 20 * PAGE_SIZE), "truncate failed");

    struct process *proc1 = spawn_process();
    struct process *proc2 = spawn_process();

    char buffer0[PAGE_SIZE] = {0};
    char buffer1[PAGE_SIZE] = "Hello world!";

    // Process 1 maps at 2 page offset and writes at 2 page offset in file
    context_switch(proc1);
    addr32 proc1_ptr1 = syscall_mmap(8 * PAGE_SIZE, PROT_WRITE | PROT_READ,
                                     MAP_SHARED, filename, 2 * PAGE_SIZE);
    pte_t *pte11 = get_pte(proc1, proc1_ptr1, true);

    testcase_assert(nommu_write(buffer1, proc1_ptr1, 0) == FAULT_MAJOR,
                    "Did not get major fault on first write to file");

    // Process 1 maps at 0 page offset and reads at 2 page offset in file

    addr32 proc1_ptr2 =
        syscall_mmap(10 * PAGE_SIZE, PROT_READ, MAP_SHARED, filename, 0) +
        2 * PAGE_SIZE;
    pte_t *pte12 = get_pte(proc1, proc1_ptr2, true);
    testcase_assert(nommu_read(buffer0, proc1_ptr2, 0) == FAULT_MINOR,
                    "Did not get minor fault on read after write to file");

    // Process 2 maps at 0 offset and reads at 2 page offset in file

    context_switch(proc2);
    addr32 proc2_ptr1 = syscall_mmap(20 * PAGE_SIZE, PROT_WRITE | PROT_READ,
                                     MAP_SHARED, filename, 0) +
                        2 * PAGE_SIZE;
    pte_t *pte21 = get_pte(proc2, proc2_ptr1, true);
    testcase_assert(nommu_read(buffer0, proc2_ptr1, 0) == FAULT_MINOR,
                    "Did not get minor fault on read after write to file");

    // After swapping out pte12's reference to the page, process 2 should get
    // a major fault on a write to the same page (page out should also succeed)

    testcase_assert(!page_out_shared_page(
                        phys_to_page(pte12->base_addr << NUM_OFFSET_BITS)),
                    "Page out failed?");

    testcase_assert(nommu_write("Somebody once told me", proc2_ptr1, 0) ==
                        FAULT_MAJOR,
                    "Did not get major fault on write after swapout");

    // Process 1's PTE 1 needs to be present and dirty

    // Process 1's PTEs need to be not present and not dirty

    testcase_assert(!pte21->present, "Process 2's PTE should be not present");
    testcase_assert(pte21->dirty, "Process 2's PTE should be dirty");
    testcase_assert(!pte11->present, "Process 1's PTE 1 should be not present");
    testcase_assert(!pte11->dirty, "Process 1's PTE 1 should be clean");
    testcase_assert(!pte12->present, "Process 1's PTE 2 should be not present");
    testcase_assert(!pte12->dirty, "Process 1's PTE 2 should be clean");

    kill_process(proc1);
    kill_process(proc2);

    fclose(tmpfp);
    unlink(filename);

    return SUCCESS;
}

static int msync_test(void) {
    char filename[] = "./tmp/msync";
    FILE *tmpfp = fopen(filename, "w+");
    testcase_assert(tmpfp, "Failed to open %s", filename);

    testcase_assert(!truncate(filename, 20 * PAGE_SIZE), "truncate failed");

    struct process *proc1 = spawn_process();
    struct process *proc2 = spawn_process();

    char buffer0[PAGE_SIZE] = {0};
    char buffer1[PAGE_SIZE] = "Hello world!";

    // Process 1 maps at 2 page offset and writes at 2 page offset in file
    context_switch(proc1);
    addr32 proc1_ptr1 = syscall_mmap(8 * PAGE_SIZE, PROT_WRITE | PROT_READ,
                                     MAP_SHARED, filename, 2 * PAGE_SIZE);
    pte_t *pte11 = get_pte(proc1, proc1_ptr1, true);

    testcase_assert(nommu_write(buffer1, proc1_ptr1, 0) == FAULT_MAJOR,
                    "Did not get major fault on first write to file");

    // Process 1 maps at 0 page offset and reads at 2 page offset in file

    addr32 proc1_ptr2 =
        syscall_mmap(10 * PAGE_SIZE, PROT_READ, MAP_SHARED, filename, 0) +
        2 * PAGE_SIZE;
    pte_t *pte12 = get_pte(proc1, proc1_ptr2, true);
    testcase_assert(nommu_read(buffer0, proc1_ptr2, 0) == FAULT_MINOR,
                    "Did not get minor fault on read after write to file");

    // Process 2 maps at 0 offset and reads at 2 page offset in file

    context_switch(proc2);
    addr32 proc2_ptr1 = syscall_mmap(20 * PAGE_SIZE, PROT_WRITE | PROT_READ,
                                     MAP_SHARED, filename, 0) +
                        2 * PAGE_SIZE;
    pte_t *pte21 = get_pte(proc2, proc2_ptr1, true);
    testcase_assert(nommu_read(buffer0, proc2_ptr1, 0) == FAULT_MINOR,
                    "Did not get minor fault on read after write to file");

    // After calling msync, process 2 should get a minor (dirty) fault
    // an write to the same page (page out should also succeed)

    testcase_assert(!syscall_msync(proc2_ptr1), "msync failed on valid VMA");

    testcase_assert(nommu_write("Somebody once told me", proc2_ptr1, 0) ==
                        FAULT_MINOR,
                    "Did not get minor fault on write after swapout");

    // All PTEs should now be clean and present

    testcase_assert(pte21->present, "Process 2's PTE should be present");
    testcase_assert(pte21->dirty, "Process 2's PTE should be dirty");
    testcase_assert(pte11->present, "Process 1's PTE 1 should be present");
    testcase_assert(!pte11->dirty, "Process 1's PTE 1 should be clean");
    testcase_assert(pte12->present, "Process 1's PTE 2 should be present");
    testcase_assert(!pte12->dirty, "Process 1's PTE 2 should be clean");

    // Backing physical file needs to be updated to what was written before the
    // last call to msync

    char file_buffer[PAGE_SIZE] = {0};
    fseek(tmpfp, 2 * PAGE_SIZE, SEEK_SET);
    testcase_assert(fread(file_buffer, PAGE_SIZE, 1, tmpfp) == 1,
                    "fread failed");
    testcase_assert(!strcmp(file_buffer, "Hello world!"),
                    "Backing physical file needs to be updated to what was "
                    "last written before msync");

    kill_process(proc1);
    kill_process(proc2);

    fclose(tmpfp);
    unlink(filename);

    return SUCCESS;
}

/*****************************************************************************
 *                                                                           *
 *                 END TEST CASE DEFINITIONS                                 *
 *                                                                           *
 ****************************************************************************/

static testcase *get_testcase_pointer(unsigned test_number) {
    return all_testcases[test_number - 1];
}

static int parse_test_number(int argc, char *argv[]) {
    if (argc > 2) {
        goto USAGE_FAILURE;
    }
    char *check_err = NULL;
    if (argc == 2) {
        unsigned long long test_number = strtoull(argv[1], &check_err, 10);
        if (*check_err) {
            goto USAGE_FAILURE;
        }
        if (!test_number || test_number > number_of_testcases) {
            fprintf(stderr, "Invalid test number.\n");
            goto USAGE_FAILURE;
        }
        return test_number;
    }
    return 0;
USAGE_FAILURE:
    fprintf(stderr, "Usage: %s [test_number]\n", argv[0]);
    exit(-1);
}

static int fork_and_run_all(char const *program_name) {
    bool did_fail[MAX_TEST_NUMBER];
    memset(did_fail, 0, sizeof(bool) * MAX_TEST_NUMBER);
    int incorrect_count = 0;

    fprintf(stderr, "======================================="
                    "=======================================\n");
    for (size_t i = 1; i <= number_of_testcases; ++i) {
        testcase *test = get_testcase_pointer(i);
        if (!test) {
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            self_exec_one(program_name, i);
            perror("execl");
        } else {
            int status = 0;
            waitpid(pid, &status, 0);
            if (!WIFEXITED(status) || WEXITSTATUS(status) != SUCCESS) {
                if (WIFSIGNALED(status)) {
                    fprintf(stderr, "Killed by signal %s\n",
                            strsignal(WTERMSIG(status)));
                }
                ++incorrect_count;
                did_fail[i - 1] = true;
            }
        }
    }
    fprintf(stderr, "======================================="
                    "=======================================\n");
    if (incorrect_count) {
        fprintf(stderr, "%d test failures!\n", incorrect_count);
        for (unsigned i = 1; i <= MAX_TEST_NUMBER; ++i) {
            if (did_fail[i - 1]) {
                fprintf(stderr, "Test %u: %s\n", i, all_test_labels[i - 1]);
            }
        }
    } else {
        fprintf(stderr, "Success!\n");
    }
    return incorrect_count;
}

static int self_exec_one(char const *program_name, unsigned number) {
    char number_str[32] = {0};
    snprintf(number_str, 32, "%u", number);
    return execl(program_name, program_name, number_str, NULL);
}
