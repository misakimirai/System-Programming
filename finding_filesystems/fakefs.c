/**
* Finding Filesystems Lab
* CS 241 - Spring 2018
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif

#include <unistd.h>

#include "dictionary.h"
#include "minixfs.h"
#include "minixfs_utils.h"
#include "vector.h"
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fakefs.h"

#define HOOK(fname) orig_##fname = dlsym(RTLD_NEXT, #fname)
#define HOOK2(fname, realname) orig_##fname = dlsym(RTLD_NEXT, #realname)

#define FD_UNIMPLEMENTED(fd, error)                           \
    if (!disable_hooks && dictionary_contains(fd_map, &fd)) { \
        return error;                                         \
    }

#define PATH_UNIMPLEMENTED(path, error)                           \
    do {                                                          \
        char *PATH_UNIMPLEMENTED_full_path = get_full_path(path); \
        if (PATH_UNIMPLEMENTED_full_path &&                       \
            is_virtual_file(PATH_UNIMPLEMENTED_full_path)) {      \
            free(PATH_UNIMPLEMENTED_full_path);                   \
            return error;                                         \
        }                                                         \
        free(PATH_UNIMPLEMENTED_full_path);                       \
    } while (0)

static file_system *fs;
static char *cwd;
static char *root;
static int disable_hooks = 1;
static dictionary *fd_map;
static dictionary *dir_fd_map;

char *load_LD(char *so_name) {
    char *preload = getenv("LD_PRELOAD");

    char *custom_preload = NULL;
    asprintf(&custom_preload, "./%s%s%s", so_name, preload ? ":" : "",
             preload ? preload : "");

    int err = setenv("LD_PRELOAD", custom_preload, 1);
    if (err) {
        fprintf(stderr, "Unable to set environment\n");
        exit(EXIT_FAILURE);
    }
    return custom_preload;
}

int main(int argc, char **argv) {
    if (argc == 3) {
        if (!strcmp(argv[1], "mkfs")) {
            disable_hooks = 1;
            minixfs_mkfs(argv[2]);
            return 0;
        }
    }
    if (argc < 3) {
        fprintf(stderr, "%s <minix_file> <command> [command_args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *custom_preload = load_LD("fakefs.so");
    setenv("MINIX_ROOT", argv[1], 1);

    execvp(argv[2], argv + 2);
    perror("Exec failed");

    free(custom_preload);
}

char *join(const char *root, const char *path) {
    char *root_tmp = strdup(root);
    if (root_tmp[strlen(root) - 1] == '/') {
        root_tmp[strlen(root) - 1] = 0;
    }

    char *ret = NULL;
    asprintf(&ret, "%s/%s", root_tmp, path);
    free(root_tmp);
    return ret;
}

int is_virtual_file(const char *file) {
    return strlen(file) >= strlen(cwd) && !strncmp(file, cwd, strlen(cwd)) &&
           !strncmp(root, file + strlen(cwd) + 1, strlen(root));
}

int __access(const char *pathname, int mode) {
    fprintf(stderr, "called access %s\n", pathname);
    if (disable_hooks)
        return orig_access(pathname, mode);

    char *full_pathname = NULL;
    if (pathname[0] != '/')
        full_pathname = join(cwd, pathname);
    else
        full_pathname = strdup(pathname);

    if (is_virtual_file(full_pathname)) {
        char *virtual_path =
            strdup(full_pathname + strlen(cwd) + 1 + strlen(root));
        fprintf(stderr, "called vaccess %s\n", virtual_path);
        return minixfs_access(fs, pathname, mode);
    }

    return orig_access(pathname, mode);
}

static char *get_full_path(const char *pathname) {
    if (!root)
        return NULL;
    char *root_in_path = strstr(pathname, root);
    if (root_in_path) {
        if (root_in_path == pathname) {
            char *real_path = realpath(".", NULL);
            char *full_path = NULL;
            asprintf(&full_path, "%s/%s", real_path, pathname);
            free(real_path);
            return full_path;
        } else {
            if (root_in_path[-1] != '/')
                return NULL;

            size_t length = root_in_path - pathname;
            char *prefix_path = malloc(length + 1);
            strncpy(prefix_path, pathname, length);
            prefix_path[length] = 0;
            char *real_prefix = realpath(prefix_path, NULL);
            if (!real_prefix) {
                free(prefix_path);
                return NULL;
            }
            char *full_path = NULL;
            asprintf(&full_path, "%s/%s", real_prefix, root_in_path);
            free(prefix_path);
            free(real_prefix);
            return full_path;
        }
    }
    return NULL;
}

/* START CORE */
int creat(const char *pathname, mode_t mode) {
    if (disable_hooks)
        return orig_creat(pathname, mode);

    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        free(full_pathname);
        return open(pathname, O_CREAT, mode);
    }
    free(full_pathname);
    return orig_creat(pathname, mode);
}

int open(const char *pathname, int flags, ...) {
    va_list a_list;
    va_start(a_list, flags);
    if (disable_hooks) {
        if ((flags & O_CREAT) == O_CREAT) {
            mode_t m = va_arg(a_list, mode_t);
            return orig_open(pathname, flags, m);
        }
        return orig_open(pathname, flags);
    }

    int error = 0;
    int v_fd = -1;

    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        char *virtual_path =
            strdup(full_pathname + strlen(cwd) + 1 + strlen(root));
        if (strlen(virtual_path) == 0) {
            free(virtual_path);
            virtual_path = strdup("/");
        }
        int error = minixfs_open(fs, virtual_path, flags);
        if (error >= 0) {
            v_fd = orig_open("/dev/null", O_RDONLY);

            fakefile *file = malloc(sizeof(fakefile));
            file->fd = v_fd;
            file->path = virtual_path;
            file->offset = 0;
            file->refcount = 1;
            file->flags = 0;

            dictionary_set(fd_map, &v_fd, file);

        } else {
            free(virtual_path);
        }
    }

    free(full_pathname);

    if (error >= 0 && v_fd >= 0)
        return v_fd;
    if (error < 0)
        return error;

    if ((flags & O_CREAT) == O_CREAT) {
        mode_t m = va_arg(a_list, mode_t);
        return orig_open(pathname, flags, m);
    }
    return orig_open(pathname, flags);
}

ssize_t read(int fd, void *buf, size_t count) {
    if (disable_hooks)
        return orig_read(fd, buf, count);

    if (dictionary_contains(fd_map, &fd)) {
        fakefile *file = *(dictionary_at(fd_map, &fd).value);
        ssize_t m_read =
            minixfs_read(fs, file->path, buf, count, &(file->offset));
        return m_read;
    }

    return orig_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (disable_hooks)
        return orig_write(fd, buf, count);
    if (dictionary_contains(fd_map, &fd)) {
        fakefile *file = *(dictionary_at(fd_map, &fd).value);
        ssize_t res =
            minixfs_write(fs, file->path, buf, count, &(file->offset));
        return res;
    }
    return orig_write(fd, buf, count);
}
int close(int fd) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_close(fd);

    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    dictionary_remove(fd_map, &fd);
    file->refcount--;
    if (!file->refcount)
        destroy_fakefile(file);
    return 0;
}

off_t lseek(int fd, off_t offset, int whence) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_lseek(fd, offset, whence);

    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    struct stat buf;

    switch (whence) {
    case SEEK_SET:
        file->offset = offset;
        break;
    case SEEK_CUR:
        file->offset += offset;
        break;
    case SEEK_END:
        if (fstat(fd, &buf) >= 0)
            file->offset = buf.st_size + offset;
        else
            return -1;
        break;
    default:
        break;
    }
    return file->offset;
}

int unlink(const char *pathname) {
    if (disable_hooks)
        return orig_unlink(pathname);
    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        char *virtual_path =
            strdup(full_pathname + strlen(cwd) + 1 + strlen(root));
        int retval = minixfs_unlink(fs, virtual_path);
        free(virtual_path);
        free(full_pathname);
        return retval;
    }
    free(full_pathname);
    return orig_unlink(pathname);
}

int mkdir(const char *pathname, mode_t mode) {
    if (disable_hooks)
        return orig_mkdir(pathname, mode);
    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        char *virtual_path =
            strdup(full_pathname + strlen(cwd) + 1 + strlen(root));
        int retval = minixfs_mkdir(fs, virtual_path, (int)mode);
        free(virtual_path);
        free(full_pathname);
        return retval;
    }
    free(full_pathname);
    return orig_mkdir(pathname, mode);
}

int chmod(const char *pathname, mode_t mode) {
    int fd = open(pathname, O_WRONLY);
    int retval = fchmod(fd, mode);
    close(fd);
    return retval;
}

int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        int retval = chmod(pathname, mode);
        free(full_pathname);
        return retval;
    }

    free(full_pathname);
    return orig_fchmodat(dirfd, pathname, mode, flags);
}

int fchmod(int fd, mode_t mode) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_fchmod(fd, mode);
    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    // fprintf(stderr, "[FCHMOD] %s\n", file->path);
    minixfs_chmod(fs, file->path, (int)mode);
    return 0;
}

int chown(const char *pathname, uid_t owner, gid_t group) {
    return lchown(pathname, owner, group);
}

int lchown(const char *pathname, uid_t owner, gid_t group) {
    int fd = open(pathname, O_WRONLY);
    int retval = fchown(fd, owner, group);
    close(fd);
    return retval;
}

int fchown(int fd, uid_t owner, gid_t group) {
    if (disable_hooks || !dictionary_contains(fd_map, &fd))
        return orig_fchown(fd, owner, group);
    fakefile *file = *(dictionary_at(fd_map, &fd).value);
    // fprintf(stderr, "[FCHOWN] %s %d\n", file->path, (int)owner);
    minixfs_chown(fs, file->path, owner, group);
    return 0;
}

int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group,
             int flags) {
    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        char *virtual_path =
            strdup(full_pathname + strlen(cwd) + 1 + strlen(root));
        int retval = chown(pathname, owner, group);
        free(virtual_path);
        free(full_pathname);
        return retval;
    }

    return orig_fchownat(dirfd, pathname, owner, group, flags);
}

/* END CORE */

/* START DUP */

int dup2(int oldfd, int newfd) {
    if (disable_hooks || !dictionary_contains(fd_map, &oldfd))
        return orig_dup2(oldfd, newfd);

    fakefile *file = *(dictionary_at(fd_map, &oldfd).value);
    orig_dup2(oldfd, newfd);
    dictionary_set(fd_map, &newfd, file);
    return 1;
}

int dup(int oldfd) {
    if (disable_hooks || !dictionary_contains(fd_map, &oldfd))
        return orig_dup(oldfd);

    fakefile *file = *(dictionary_at(fd_map, &oldfd).value);
    int newfd = orig_open("/dev/null", file->flags);
    if (newfd > 0) {
        dictionary_set(fd_map, &newfd, file);
        return newfd;
    }
    return newfd;
}
/* END DUP */

int fsync(int fd) {
    if (dictionary_contains(fd_map, &fd))
        return 0;
    return orig_fsync(fd);
}

int fdatasync(int fd) {
    if (dictionary_contains(fd_map, &fd))
        return 0;
    return orig_fdatasync(fd);
}

/* START STAT */

static int fstat_common(int fd, struct stat *buf, int *retval) {
    if (dictionary_contains(fd_map, &fd)) {
        fakefile *file = *(dictionary_at(fd_map, &fd).value);
        *retval = minixfs_stat(fs, file->path, buf);
        return 1;
    }
    return 0;
}
int __xstat64(int ver, const char *pathname, struct stat64 *buf) {
    return __lxstat64(ver, pathname, buf);
}
int __lxstat64(int ver, const char *pathname, struct stat64 *buf) {
    char *full_path = get_full_path(pathname);
    if (full_path) {
        int fd = open(full_path, O_RDONLY);
        int retval = __fxstat64(ver, fd, buf);
        close(fd);
        free(full_path);
        return retval;
    }
    return orig_lstat64(ver, pathname, buf);
}
int __fxstat64(int ver, int fd, struct stat64 *buf) {
    if (disable_hooks)
        return orig_fstat64(ver, fd, buf);
    int retval;
    int hooked = fstat_common(fd, (struct stat *)buf, &retval);
    if (hooked)
        return retval;

    return orig_fstat64(ver, fd, buf);
}

int __fxstatat64(int ver, int dirfd, const char *pathname, struct stat64 *buf,
                 int flags) {
    if (disable_hooks)
        return orig_fstatat64(ver, dirfd, pathname, buf, flags);
    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        int fd = open(pathname, O_RDONLY);
        int retval = __fxstat64(ver, fd, buf);
        close(fd);
        free(full_pathname);
        return retval;
    }
    free(full_pathname);
    return orig_fstatat64(ver, dirfd, pathname, buf, flags);
}

int __xstat(int ver, const char *pathname, struct stat *buf) {
    return __lxstat(ver, pathname, buf);
}
int __lxstat(int ver, const char *pathname, struct stat *buf) {
    int fd = open(pathname, O_RDONLY);
    int retval = __fxstat(ver, fd, buf);
    close(fd);
    return retval;
}
int __fxstat(int ver, int fd, struct stat *buf) {
    if (disable_hooks)
        return orig_fstat(0, fd, buf);
    int retval;
    int hooked = fstat_common(fd, buf, &retval);
    if (hooked)
        return retval;
    return orig_fstat(ver, fd, buf);
}

int __fxstatat(int ver, int dirfd, const char *pathname, struct stat *buf,
               int flags) {
    if (disable_hooks)
        return orig_fstatat(ver, dirfd, pathname, buf, flags);
    char *full_pathname = get_full_path(pathname);
    if (full_pathname && is_virtual_file(full_pathname)) {
        int fd = open(full_pathname, O_RDONLY);
        int retval = __fxstat(ver, fd, buf);
        close(fd);
        free(full_pathname);
        return retval;
    }

    free(full_pathname);
    return orig_fstatat(ver, dirfd, pathname, buf, flags);
}

int lstat(const char *pathname, struct stat *buf) {
    return __lxstat(0, pathname, buf);
}
int stat(const char *pathname, struct stat *buf) {
    return __xstat(0, pathname, buf);
}

int fstat(int fd, struct stat *buf) {
    return __fxstat(0, fd, buf);
}
/* END STAT */

/* START DIR */
DIR *fdopendir(int fd) {
    if (dictionary_contains(fd_map, &fd)) {
        fakedir *dir = malloc(sizeof(fakedir));
        dir->fd = fd;
        dir->entries_read = 0;
        dir->max_entries = 0;
        dir->entry = NULL;

        fakefile *file = *(dictionary_at(fd_map, &fd).value);

        dir->max_entries = minixfs_readdir(fs, file->path, &(dir->entry));
        if (dir->max_entries < 0) {
            free(dir);
            return NULL;
        }

        DIR *realdir = orig_opendir("/dev/");
        dictionary_set(dir_fd_map, realdir, dir);

        return realdir;
    }
    return orig_fdopendir(fd);
}
DIR *opendir(const char *name) {
    char *full_path = get_full_path(name);
    if (full_path)
        name = full_path;

    int fd = open(name, O_DIRECTORY);
    if (full_path)
        free(full_path);

    DIR *dirp = fdopendir(fd);
    return dirp;
}

struct dirent64 *readdir64(DIR *dirp) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        return (struct dirent64 *)readdir(dirp);
    }
    return orig_readdir64(dirp);
}
struct dirent *readdir(DIR *dirp) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        fakedir *dir = *(dictionary_at(dir_fd_map, dirp)).value;
        if (dir->entries_read >= dir->max_entries) {
            return NULL;
        }
        struct dirent *entry = &(dir->entry[dir->entries_read]);
        dir->entries_read++;
        return entry;
    }
    return orig_readdir(dirp);
}
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        fakedir *dir = *(dictionary_at(dir_fd_map, dirp)).value;
        if (dir->entries_read >= dir->max_entries) {
            *result = NULL;
            return 0;
        }
        struct dirent *entry = &(dir->entry[dir->entries_read]);
        dir->entries_read++;
        *result = entry;
        return 0;
    }
    return orig_readdir_r(dirp, entry, result);
}
int closedir(DIR *dirp) {
    if (dictionary_contains(dir_fd_map, dirp)) {
        fakedir *dir = *(dictionary_at(dir_fd_map, dirp)).value;
        close(dir->fd);
        dictionary_remove(dir_fd_map, dirp);
        destroy_fakedir(dir);
        return 0;
    }
    return orig_closedir(dirp);
}
/* END DIR */

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
    FD_UNIMPLEMENTED(fd, NULL);
    return orig_mmap(addr, length, prot, flags, fd, offset);
}

static void destroy_fakefile(fakefile *f) {
    free(f->path);
    free(f);
}

static void destroy_fakedir(fakedir *d) {
    free(d->entry);
    free(d);
}

void __attribute__((destructor)) fs_fini(void) {
    // fprintf(stderr, "Goodbye!\n");
    free(cwd);
    if (root) {
        vector *keys = dictionary_keys(fd_map);
        VECTOR_FOR_EACH(keys, elem, { close(*(int *)elem); });
        vector_destroy(keys);
        // dictionary_destroy(fd_map);

        keys = dictionary_keys(fd_map);
        VECTOR_FOR_EACH(keys, elem, { closedir(*(DIR **)elem); });
        vector_destroy(keys);
        // dictionary_destroy(dir_fd_map);
        disable_hooks = 1;
        // close_fs(&fs);
    }
}

void __attribute__((constructor)) fs_init(void) {
    HOOK2(access, __access);

    HOOK(creat);
    HOOK(open);
    HOOK(read);
    HOOK(write);
    HOOK(close);
    HOOK(lseek);
    HOOK(unlink);
    HOOK(mkdir);
    HOOK(fchmod);
    HOOK(fchmodat);
    HOOK(fchown);
    HOOK(fchownat);

    HOOK(fsync);
    HOOK(fdatasync);

    HOOK(dup);
    HOOK(dup2);

    HOOK2(stat, __xstat);
    HOOK2(lstat, __lxstat);
    HOOK2(fstat, __fxstat);
    HOOK2(fstatat, __fxstatat);
    HOOK2(stat64, __xstat64);
    HOOK2(lstat64, __lxstat64);
    HOOK2(fstat64, __fxstat64);
    HOOK2(fstatat64, __fxstatat64);

    HOOK(fdopendir);
    HOOK(opendir);
    HOOK(readdir);
    HOOK(readdir64);
    HOOK(closedir);

    HOOK(mmap);

    root = getenv("MINIX_ROOT");
    if (root) {
        fs = open_fs(root);
        fd_map = int_to_shallow_dictionary_create();
        dir_fd_map = shallow_to_shallow_dictionary_create();
    }

    cwd = getcwd(NULL, 0);
    // fprintf(stderr, "Set cwd:%s\n", cwd);
    disable_hooks = 0;
}
