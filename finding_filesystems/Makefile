OBJS_DIR = .objs

# define all the student executables
MINIX_EXE=fakefs
EXES_STUDENT=$(MINIX_EXE)
EXES_TEST=minixfs_test
# list object file dependencies for each
OBJS_MINIX=minixfs.o minixfs_provided.o minixfs_utils.o
OBJS_TEST=$(OBJS_MINIX) minixfs_test.o
OBJS_FAKEFS=fakefs.o 
OBJS_ALL=$(OBJS_MINIX) $(OBJS_FAKEFS)
FS_FILES=minixfs.c minixminixfs_provided.c minixfs_utils.c

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-result -Wmissing-declarations -Wmissing-variable-declarations
INC=-I./includes/
CFLAGS_DEBUG   = -O0 $(WARNINGS) $(INC) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -fPIC
CFLAGS_RELEASE = -O0 $(WARNINGS) $(INC) -std=c99 -c -MMD -MP -D_GNU_SOURCE -fPIC

# set up linker
LD = clang
LDFLAGS = -lrt -ldl -Llibs/ -lprovided 

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
IS_VM=$(shell hostname | grep "cs241")
VM_OVERRIDE=$(shell echo $$HOSTNAME)
ifeq ($(IS_VM),)
ifneq ($(VM_OVERRIDE),cs241grader)
$(error This assignment must be compiled on the CS241 VMs)
endif
endif


.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: debug
.PHONY: release

release: $(EXES_STUDENT) fakefs.so $(EXES_TEST)
debug:   clean $(EXES_STUDENT:%=%-debug) fakefs-debug.so $(EXES_TEST:%=%-debug) 

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@


# exes

fakefs.so: $(OBJS_MINIX:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) fakefs.c $^ -shared -fPIC $(LDFLAGS) $(INC) -o fakefs.so

fakefs-debug.so: $(OBJS_MINIX:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) fakefs.c $^ -shared -fPIC -g $(LDFLAGS) $(INC) -o fakefs.so

$(MINIX_EXE): $(OBJS_ALL:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(MINIX_EXE)-debug: $(OBJS_ALL:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXES_TEST): $(OBJS_TEST:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ -o minixfs_test

$(EXES_TEST)-debug: $(OBJS_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ -o minixfs_test-debug

.PHONY: clean
clean:
	-rm -rf .objs fakefs.so $(EXES_STUDENT) $(EXES_STUDENT:%=%-debug) $(EXES_TEST) $(EXES_TEST:%=%-debug)

.PHONY: reset
reset:
	-rm -rf test_fs/
	make fs-get

.PHONY: testfs
testfs: test.fs
test.fs:
	svn export https://subversion.ews.illinois.edu/svn/sp18-cs241/_shared/finding_filesystems/test.fs
