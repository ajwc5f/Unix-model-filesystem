Implementation of a filesystem that interfaces to a Block Store.

#### The following are the sizing constraints of the Block Store:
1. The total number of blocks is 2^16.
2. The size of blocks is fixed at 512 bytes.
3. Some of these blocks are pre-allocated to the free block map.

#### Filesystem is capable of:
 - Format an F16FS file
 - Mount and unmount an F16FS file
 - Create directories and regular files
 - Open, close, and seek files
 - Read and write to files
 - Remove files
 - List directory contents

#### Regarding inodes:
Each inode is exactly 64 bytes. The general inode structure is as follows:
 - Metadata / ACL: bytes 0-47
 - Data Block Pointers: bytes 48-63

The inode table is 16 KB in size, i.e., 32 data blocks worth of inodes. Inodes use 16-bit addressing for
block references. The filesystem root directory is located at the first inode.

#### Regarding block pointers:
Inodes contain 8 block pointers: 6 direct, 1 indirect, and 1 double indirect. Direct pointers are the id numbers of the data blocks that make up the file. Indirect pointers point to a data block
that is made of direct pointers. A double indirect pointer points to a block of data that is made of indirect
pointers. Indirect and double indirect pointers greatly increase the maximum size of a file while the direct
pointers allow for quick and simple access to small files.

#### Regular File Block Structure:
The regular file blocks have no structure, they are pure data.

#### Directory File Block Structure:
To simplify the filesystem, directory files are limited to 7 entries. Each entry consista of:
1. The file’s name: 64 bytes
2. The file’s inode number: 1 byte
This allows a directory to be contained on a single data block. The remaining space on the directory’s data
block is allocated to Metadata.

#### File Descriptors:
The filesystem is using file descriptors to manage open files. The filesystem is limited to 256
file descriptors . Each file descriptor tracks a single read/write position for that descriptor.  
Files can support multiple descriptors to the same file, but with different read/write positions. Seeking allows the user to
move the read/write position to where they want. 

#### Milestone 1

1. fs_format
2. fs_mount
3. fs_unmount

#### Milestone 2

1. fs_create
2. fs_get_dir
3. fs_open
4. fs_close

#### Milestone 3

1. fs_write
2. fs_read
3. fs_seek
