/**
* Finding Filesystems Lab
* CS 241 - Spring 2018
*/

//----------------------------------
// INTERNAL FUNCTIONS:
//----------------------------------

/*
*  Makes a directory located at path
*    Returns error if that directory already exists
*/
int minixfs_mkdir(file_system *fs, char *path, int mode);

/*
*  Initializes a filesystem on disk with filename args.
*/
void minixfs_mkfs(char *args);

/*
*  Takes a valid file-filesystem and opens it
*  While setting up the abstraction
*/
file_system *open_fs(const char *path);

/*
*  Properly writes the file back to disk
*  And stores cleans up the dangling pointer
*/
void close_fs(file_system **fs_pointer);

/*
 * The following set of functions are mostly likely not needed to complete this
 * lab
 * You should only be using the functions in the section above.
 *
 * We have given you access to these functions in case you are interested in
 * seeing how all the magic actually works.
 */

int minixfs_open(file_system *fs, const char *path, int flags);
int minixfs_unlink(file_system *fs, const char *path);
int minixfs_access(file_system *fs, const char *path, int mode);

/*
*  Sets the bytemap after the superblock to the boolean in used
*  No operation if you give an invalid datablock number
*  Used is any valid boolean
*/
void set_data_used(file_system *fs_pointer, data_block_number data_number,
                   int used);

/*
*  Returns whether or not a datablock is being used.
*  No operation if you give an invalid datablock number
*/
int get_data_used(file_system *fs_pointer, data_block_number data_number);

/*
*  Initializes an inode to a file
*  Sets the group, owner, permissions to the parent
*  Hard Link count initialize to 1
*/
void init_inode(inode *parent, inode *init);

/*
*  Set an inode to be unused for future files
*/
void free_inode(file_system *fs_pointer, inode *node);

/*
*  Set a data block to be unused for future files
*/
void free_data_block(file_system *fs_pointer, data_block_number data_number);

/*
*  Returns the inode number of an unused inode
*  -1 if there are no more inodes in the system
*/
inode_number first_unused_inode(file_system *fs_pointer);

/*
*  Returns the data number of an unused inode
*  -1 if there are no more data nodes
*/
data_block_number first_unused_data(file_system *fs_pointer);

/*
*  Gets the inode of the parent directory and the name of the
*  filename at the end (having a trailing slash is undefined behavior)
*  char* path = '/a/b/c'
*  return inode located at '/a/b/c'
*  *filename = 'c'
*/
inode *parent_directory(file_system *fs, const char *path,
                        const char **filename);

/*
*  Returns 1 if the string is a valid filename
*  Error if passed null
*/
int valid_filename(const char *filename);
