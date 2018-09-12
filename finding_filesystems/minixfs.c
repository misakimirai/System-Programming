/**
* Finding Filesystems Lab
* CS 241 - Spring 2018
*/

#include "minixfs.h"
#include <errno.h>
#include <math.h>
#include <string.h>

/*
* partners: chenp2 my5 meishan2 yubocao2
*/

void * get_block_start(file_system * fs, inode * file, uint64_t block_index);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
 // Thar she blows!
 inode * file = get_inode(fs, path);
 if (file == NULL) {
   errno = ENOENT;
   return -1;
 }
 int type = file->mode >> RWX_BITS_NUMBER;
 file->mode = new_permissions;
 file->mode |= type << RWX_BITS_NUMBER;
 clock_gettime(CLOCK_REALTIME, &file->ctim);
 return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
 // Land ahoy!
 inode * file = get_inode(fs, path);
 if (file == NULL) {
   errno = ENOENT;
   return -1;
 }
 if (owner != ((uid_t)-1))
   file->uid = owner;
 if (group != ((gid_t)-1))
   file->gid = group;
 clock_gettime(CLOCK_REALTIME, &file->ctim);
 return 0;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                     size_t count, off_t *off) {
 // X marks the spot
 uint64_t least_size = count + *off;
 if (least_size > (sizeof(data_block) * (NUM_DIRECT_INODES + NUM_INDIRECT_INODES))) {
   errno = ENOSPC;
   return -1;
 }

 int block_count = least_size / sizeof(data_block);
 if(0 == (least_size % sizeof(data_block))){
   block_count++;
 }
 if (minixfs_min_blockcount(fs, path, block_count) == -1) {
   errno = ENOSPC;
   return -1;
 }
 inode * file = get_inode(fs, path);


 size_t block_size = sizeof(data_block);
 uint64_t written = 0;
 uint64_t block_index = (*off / block_size);
 size_t write_size = block_size - (*off) % block_size;
 write_size = (write_size < count ? write_size : count);
 char * start = (char *)get_block_start(fs, file, block_index) + (*off) % block_size;
 memmove(start, buf, write_size);
 written += write_size;
 block_index ++;
 *off += write_size;
 while (written < count) {
   write_size = ((count - written) > block_size ? block_size : (count - written));
   start = (char *)get_block_start(fs, file, block_index);
   memmove(start, buf + written, write_size);
   written += write_size;
   *off += write_size;
   block_index++;
 }
 file->size = (*off + count > file->size) ? (*off + count) : file->size;

 clock_gettime(CLOCK_REALTIME, &file->mtim);
 clock_gettime(CLOCK_REALTIME, &file->atim);
 return (ssize_t)written;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                    off_t *off) {
 // 'ere be treasure!
 inode * file = get_inode(fs, path);
 if (file == NULL) {
   errno = ENOENT;
   return -1;
 }

 size_t block_size = sizeof(data_block);
 uint64_t size = file->size;
 if ((unsigned long)*off > size)
   return 0;
 count = ((*off + count) > size ? (size - *off) : count);
 uint64_t red = 0;
 uint64_t block_index = (*off / block_size);
 size_t read_size = block_size - (*off) % block_size;
 read_size = (read_size < count ? read_size : count); // first block
 char * start = (char *)get_block_start(fs, file, block_index) + (*off) % block_size;
 read_size = ((*off + read_size) < size ? read_size : (size - *off)); // check pass end
 memmove(buf, start, read_size);
 red += read_size;
 *off += read_size;
 block_index++;
 while (red < count) {
   read_size = ((count - red) > block_size ? block_size : (count - red));
   start = (char *)get_block_start(fs, file, block_index);
   memmove(buf + red, start, read_size);
   red += read_size;
   *off += read_size;
   block_index++;
 }



 clock_gettime(CLOCK_REALTIME, &file->mtim);
 clock_gettime(CLOCK_REALTIME, &file->atim);
 return red;
}


void * get_block_start(file_system * fs, inode * file, uint64_t block_index) {
 data_block_number * block_array;
 if (block_index < NUM_DIRECT_INODES) {
   block_array = file->direct;
 } else {
   block_index -= NUM_DIRECT_INODES;
   block_array = (data_block_number *)(fs->data_root + file->indirect);
 }
 return (void *)(fs->data_root + block_array[block_index]);
}
