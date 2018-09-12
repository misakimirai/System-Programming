OBJS_DIR = .objs

# define all the student executables
MMAP_TEST=mman_tests
MMAP_CUSTOM_TEST=mman_custom_tests
EXES_STUDENT=$(MMAP_TEST)


# list object file dependencies for each
OBJS_MMAP_TEST=mman_fixed_tests.o syscalls.o page_cache.o disk.o virtual_memory.o physical_memory.o page_table.o kernel.o page_fault_helpers.o nommu.o
OBJS_MMAP_CUSTOM_TEST=mman_tests.o syscalls.o page_cache.o disk.o virtual_memory.o physical_memory.o page_table.o kernel.o page_fault_helpers.o nommu.o

# set up compiler
CC = gcc 
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-result
CFLAGS_DEBUG   = -O0 $(WARNINGS) -g3 -gdwarf-2 -std=c99 -c -MMD -MP -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE
CFLAGS_TSAN    = $(CFLAGS_DEBUG)

# set up linker
LD = gcc
LDFLAGS = 

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
# IS_VM=$(shell hostname | grep "cs241")
#
# ifeq ($(IS_VM),)
# $(error This assignment must be compiled on the CS241 VMs)
# endif

.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: debug
.PHONY: release
.PHONY: custom_debug
.PHONY: custom

release:      $(EXES_STUDENT)
debug:        clean $(EXES_STUDENT:%=%-debug)
custom:       clean $(EXES_STUDENT) $(MMAP_CUSTOM_TEST)
custom_debug: clean $(EXES_STUDENT:%=%-debug) $(MMAP_CUSTOM_TEST:%=%-debug)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need a triple of exe and exe-debug and exe-tsan for each exe (other
# than provided exes)

$(MMAP_TEST): $(OBJS_MMAP_TEST:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(MMAP_TEST)-debug: $(OBJS_MMAP_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(MMAP_CUSTOM_TEST): $(OBJS_MMAP_CUSTOM_TEST:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(MMAP_CUSTOM_TEST)-debug: $(OBJS_MMAP_CUSTOM_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@
.PHONY: clean
clean:
	-rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-debug) $(MMAP_CUSTOM_TEST) $(MMAP_CUSTOM_TEST:%=%-debug)
