#include <block_store.h>
#include <bitmap.h>

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "f16fs.h"
#include "backend.h"

///
/// Formats (and mounts) an S16FS file for use
/// \param fname The file to format
/// \return Mounted S16FS object, NULL on error
///
F16FS_t *fs_format(const char *path) {
    return ready_file(path, true);
}
///
/// Mounts an F16FS object and prepares it for use
/// \param fname The file to mount
/// \return Mounted F16FS object, NULL on error
///
F16FS_t *fs_mount(const char *path) {
    return ready_file(path, false);
}
///
/// Unmounts the given object and frees all related resources
/// \param fs The F16FS object to unmount
/// \return 0 on success, < 0 on failure
///
int fs_unmount(F16FS_t *fs) {
    if (fs) {
        block_store_close(fs->bs);
        bitmap_destroy(fs->fd_table.fd_status);
        free(fs);
        return 0;
    }
    return -1;
}

///
/// Creates a new file at the specified location
///   Directories along the path that do not exist are not created
/// \param fs The F16FS containing the file
/// \param path Absolute path to file to create
/// \param type Type of file to create (regular/directory)
/// \return 0 on success, < 0 on failure
///
int fs_create(F16FS_t *fs, const char *path, file_t type) {
    if (fs && path) {
        if (type == FS_REGULAR || type == FS_DIRECTORY) {
            // WHOOPS. Should make sure desired file doesn't already exist.
            // Just going to jam it here.
            result_t file_status;
            locate_file(fs, path, &file_status);
            if (file_status.success && !file_status.found) {
                // alrighty. Need to find the file. And by the file I mean the parent.
                // locate_file doesn't really handle finding the parent if the file doesn't exist
                // So I can't just dump finding this file. Have to search for parent.
                // So, kick off the file finder. If it comes back with the right flags
                // Start checking if we have inodes, the parent exists, a directory, not full
                // if it's a dir check if we have a free block.
                // Fill it all out, update parent, etc. Done!
                const size_t path_len = strnlen(path, FS_PATH_MAX);
                if (path_len != 0 && path[0] == '/' && path_len < FS_PATH_MAX) {
                    // path string is probably ok.
                    char *path_copy, *fname_copy;
                    // this breaks if it's a file at root, since we remove the slash
                    // locate_file treats it as an error
                    // Old version just worked around if if [0] was '\0'
                    // Ideally, I could just ask strndup to allocate an extra byte
                    // Then I can just shift the fname down a byte and insert the NUL there
                    // But strndup doesn't allocate the size given, it seems
                    // So we gotta go manual. Don't think this snippet will be needed elsewhere
                    // Need a malloc, memcpy, then some manual adjustment
                    // path_copy  = strndup(path, path_len);  // I checked, it's not +1. yay MallocScribble
                    path_copy = (char *) calloc(1, path_len + 2);  // NUL AND extra space
                    memcpy(path_copy, path, path_len);
                    fname_copy = strrchr(path_copy, '/');
                    if (fname_copy) {  // CANNOT be null, since we validated [0] as a possibility... but just in case
                        //*fname_copy = '\0';  // heh, split strings, now I have a path to parent AND fname
                        ++fname_copy;
                        const size_t fname_len = path_len - (fname_copy - path_copy);
                        memmove(fname_copy + 1, fname_copy, fname_len + 1);
                        fname_copy[0] = '\0';  // string is split into abs path (now with slash...) and fname
                        ++fname_copy;
                        if (fname_len != 0 && fname_len < (FS_FNAME_MAX - 1)) {
                            // alrighty. Hunt down parent dir.
                            // check it's actually a dir. (ooh, add to result_t!)
                            locate_file(fs, path_copy, &file_status);
                            if (file_status.success && file_status.found && file_status.type == FS_DIRECTORY) {
                                // parent exists, is a directory. Cool.
                                // (added block to locate_file if file is a dir. Handy.)
                                dir_block_t parent_dir;
                                inode_t new_inode;
                                dir_block_t new_dir;
                                uint32_t now = time(NULL);
                                // load dir, check it has space.
                                if (full_read(fs, &parent_dir, file_status.block)
                                    && parent_dir.mdata.size < DIR_REC_MAX) {
                                    // try to grab all new resources (inode, optionally data block)
                                    // if we get all that, commit it.
                                    inode_ptr_t new_inode_idx = find_free_inode(fs);
                                    //printf("INODE %d FOR %s!\n", new_inode_idx, path);
                                    if (new_inode_idx != 0) {
                                        bool success            = false;
                                        block_ptr_t new_dir_ptr = 0;
                                        switch (type) {
                                            case FS_REGULAR:
                                                // We're all good.
                                                new_inode = (inode_t){
                                                    {0, 0777, now, now, now, file_status.inode, FS_REGULAR, 1, {0}},
                                                    {0}};
                                                // I'm so deep now that my formatter is very upset with every line
                                                // inode = ready
                                                success = write_inode(fs, &new_inode, new_inode_idx);
                                                // Uhh, if that didn't work we could, worst case, have a partial inode
                                                // And that's a "file system is now kinda busted" sort of error
                                                // This is why "real" (read: modern) file systems have backups all over
                                                // (and why the occasional chkdsk is so important)
                                                break;
                                            case FS_DIRECTORY:
                                                // following line keeps being all "Expected expression"
                                                // SOMETHING is messed up SOMEWHERE.
                                                // Or it's trying to protect me by preventing new variables in a switch
                                                // Which is super undefined, but only sometimes (not in this case...)
                                                // Idk, man.
                                                // block_ptr_t new_dir_ptr = block_store_allocate(fs->bs);
                                                new_dir_ptr = block_store_allocate(fs->bs);
                                                if (new_dir_ptr != 0) {
                                                    // Resources = obtained
                                                    // write dir block first, inode is the final step
                                                    // that's more transaction-safe... but it's not like we're thread
                                                    // safe
                                                    // in the slightest (or process safe, for that matter)
                                                    new_inode = (inode_t){
                                                        {0, 0777, now, now, now, file_status.inode, FS_DIRECTORY, 1, {0}},
                                                        {new_dir_ptr, 0, 0, 0, 0, 0}};
                                                    memset(&new_dir, 0x00, sizeof(dir_block_t));
                                                    if (!(success = full_write(fs, &new_dir, new_dir_ptr)
                                                                    && write_inode(fs, &new_inode, new_inode_idx))) {
                                                        // transation: if it didn't work, release the allocated block
                                                        block_store_release(fs->bs, new_dir_ptr);
                                                    }
                                                }
                                                break;
                                            default:
                                                // HOW.
                                                break;
                                        }
                                        if (success) {
                                            // whoops. forgot the part where I actually save the file to the dir tree
                                            // Mildly important.
                                            unsigned i = 0;
                                            // This is technically a potential infinite loop. But we validated contents
                                            // earlier
                                            for (; parent_dir.entries[i].fname[0] != '\0'; ++i) {
                                            }
                                            strncpy(parent_dir.entries[i].fname, fname_copy, fname_len + 1);
                                            parent_dir.entries[i].inode = new_inode_idx;
                                            ++parent_dir.mdata.size;
                                            if (full_write(fs, &parent_dir, file_status.block)) {
                                                free(path_copy);
                                                return 0;
                                            } else {
                                                // Oh man. These surely are the end times.
                                                // Our file exists. Kinda. But not entirely.
                                                // The final tree link failed.
                                                // We SHOULD:
                                                //  Wipe inode
                                                //  Release dir block (if making a dir)
                                                // But I'm lazy. And if a write failed, why would others work?
                                                // block_store won't actually do that to us, anyway.
                                                // Like, even if the file was deleted while using it, we're mmap'd so
                                                // the kernel has no real way to tell us, as far as I know.
                                                puts("Infinite sadness. New file stuck in limbo.");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        free(path_copy);
                    }
                }
            }
        }
    }
    return -1;
}

///
/// Opens the specified file for use
///   R/W position is set to the beginning of the file (BOF)
///   Directories cannot be opened
/// \param fs The F16FS containing the file
/// \param path path to the requested file
/// \return file descriptor to the requested file, < 0 on error
///
int fs_open(F16FS_t *fs, const char *path) {
    if (fs && path) {
        // Well, find the file, log it. That's it?
        // faster to find an open descriptor, so we'll knock that out first.
        size_t open_descriptor = bitmap_ffz(fs->fd_table.fd_status);
        if (open_descriptor != SIZE_MAX) {
            result_t file_info;
            locate_file(fs, path, &file_info);
            if (file_info.success && file_info.found && file_info.type == FS_REGULAR) {
                // cool. Done.
                bitmap_set(fs->fd_table.fd_status, open_descriptor);
                fs->fd_table.fd_pos[open_descriptor]   = 0;
                fs->fd_table.fd_inode[open_descriptor] = file_info.inode;
                // ... auto-aligning assignments in cute until this happens.
                return open_descriptor;
            }
            // ... I really should be returning multiple error codes
            // I wrote the spec so you could do this and then I just don't
            // Like those error debugging macros.
            // Do as I say, not as I do.
        }
    }
    return -1;
}

///
/// Closes the given file descriptor
/// \param fs The F16FS containing the file
/// \param fd The file to close
/// \return 0 on success, < 0 on failure
///
int fs_close(F16FS_t *fs, int fd) {
    if (fs && fd < DESCRIPTOR_MAX && fd >= 0) {
        // Oh man, now I feel bad.
        // There's 0% chance bitmap will detect out of bounds and stop
        // So everyone's going to segfault if they don't range check the fd first.
        // Because in the C++ version, I could just throw.
        // Maybe I should just replace bitmap with the C++ version
        // and expose a C interface that just throws. ...not that it's really better
        // At least you'll see bitmap throwing as opposed to segfault (core dumped)
        // That's unfortunate.
        // I'm going to get so many emails.
        // if (bitmap_test(fs->fd_table.fd_status,fd)) {
        // Actually, I can just reset it. If it's not set, unsetting it doesn't do anything.
        // Bits, man.
        // But actually it fails the test since I say it was ok
        if (bitmap_test(fs->fd_table.fd_status, fd)) {
            bitmap_reset(fs->fd_table.fd_status, fd);
            return 0;
        }
    }
    return -1;
}

///
/// Populates a dyn_array with information about the files in a directory
///   Array contains up to 15 file_record_t structures
/// \param fs The F16FS containing the file
/// \param path Absolute path to the directory to inspect
/// \return dyn_array of file records, NULL on error
///
dyn_array_t *fs_get_dir(F16FS_t *fs, const char *path) {
    if (fs && path) {
        result_t search_results;
        locate_file(fs, path, &search_results);
        if (search_results.success && search_results.found && search_results.type == FS_DIRECTORY) {
            dir_block_t dir;
            if (full_read(fs, &dir, search_results.block)) {
                dyn_array_t *dir_contents = dyn_array_create(16, sizeof(file_record_t), NULL);
                if (dir_contents) {
                    inode_t file_inode;
                    file_record_t record;
                    for (int i = 0; i < DIR_REC_MAX; ++i) {
                        if (dir.entries[i].fname[0] != '\0') {
                            // Oh man, this is actually a pain. All the inodes have to be loaded. Uggghhhhh
                            if (read_inode(fs, &file_inode, dir.entries[i].inode)) {
                                record.type = (file_t) file_inode.mdata.type;
                                strncpy(record.name, dir.entries[i].fname, FS_FNAME_MAX);
                                if (!dyn_array_push_back(dir_contents, &record)) {
                                    // also broke
                                    dyn_array_destroy(dir_contents);
                                    return NULL;
                                }
                            } else {
                                // welp, SOMETHING broke.
                                dyn_array_destroy(dir_contents);
                                return NULL;
                            }
                        }
                    }
                    return dir_contents;
                }
            }
        }
    }
    return NULL;
}

///
/// Moves the R/W position of the given descriptor to the given location
///   Files cannot be seeked past EOF or before BOF (beginning of file)
///   Seeking past EOF will seek to EOF, seeking before BOF will seek to BOF
/// \param fs The F16FS containing the file
/// \param fd The descriptor to seek
/// \param offset Desired offset relative to whence
/// \param whence Position from which offset is applied
/// \return offset from BOF, < 0 on error
///
off_t fs_seek(F16FS_t *fs, int fd, off_t offset, seek_t whence) {
  if (fs == NULL || fd < 0 || fd >= DESCRIPTOR_MAX || !bitmap_test(fs->fd_table.fd_status, fd)) {
    return -1;
  }
  else {
    inode_t file_inode;
      
    if (read_inode(fs, &file_inode, fs->fd_table.fd_inode[fd])) {
      off_t pos_in_file = fs->fd_table.fd_pos[fd];
      off_t begin_of_file = 0;
      off_t end_of_file = file_inode.mdata.size;

      if (whence == FS_SEEK_SET) {
        pos_in_file = offset;
      }
      else if (whence == FS_SEEK_CUR) {
        pos_in_file = pos_in_file + offset;
      }
      else if (whence == FS_SEEK_END) {
        pos_in_file = end_of_file + offset;
      }
      else {
        return -1;  
      }
        
      if (pos_in_file < begin_of_file) {//cant seek before file
        pos_in_file = begin_of_file;
      } 
      else if (pos_in_file > end_of_file) {//cant seek after file
        pos_in_file = end_of_file;
      }

      fs->fd_table.fd_pos[fd] = pos_in_file;//update fd table

      return pos_in_file;

    }
  }
  return -1;
}

///
/// Writes data from given buffer to the file linked to the descriptor
///   Writing past EOF extends the file
///   Writing inside a file overwrites existing data
///   R/W position in incremented by the number of bytes written
///   If there is not enough free space for a full write, as much data as possible will be written
/// \param fs The F16FS containing the file
/// \param fd The file to write to
/// \param dst The buffer to read from
/// \param nbyte The number of bytes to write
/// \return number of bytes written (< nbyte IFF out of space), < 0 on error
///
ssize_t fs_write(F16FS_t *fs, int fd, const void *src, size_t nbyte) {
  //int error = 0;
  if (fs == NULL || fd < 0 || fd >= DESCRIPTOR_MAX || !bitmap_test(fs->fd_table.fd_status, fd) || src == NULL) {
    return -1;
  }
  else if (nbyte == 0) {
    return 0;
  }
  else {
    inode_t file_inode;
        
    inode_ptr_t file_inode_ptr = fs->fd_table.fd_inode[fd];//get inode from fd table
        
    if (!read_inode(fs, &file_inode, file_inode_ptr)) {
      return -2;
    }
    else {
      size_t pos_in_file = fs->fd_table.fd_pos[fd]; //pos to begin writing in file
      /*if (pos_in_file % BLOCK_SIZE == 0) {
        pos_in_file = 0;
      }*/
      pos_in_file = (pos_in_file % BLOCK_SIZE);
            
      size_t block_offset = POSITION_TO_INNER_OFFSET(pos_in_file);      
      size_t block_bytes_left = (BLOCK_SIZE - block_offset);
      size_t full_blocks;
      size_t extra_block_bytes;

      if (nbyte > block_bytes_left) {//must do some partial writing to the file
        full_blocks = (nbyte - block_bytes_left) / BLOCK_SIZE;
        extra_block_bytes = (nbyte - block_bytes_left) % BLOCK_SIZE;
      } 
      else {//we can write fully
        block_bytes_left = nbyte;
        full_blocks = 0;
        extra_block_bytes = 0;
      }

      size_t num_blocks_needed = 1 + full_blocks;
      if (extra_block_bytes) {
        num_blocks_needed++;//find num of blocks needed for writing
      }

      block_ptr_t needed_block_ptrs[num_blocks_needed];
      for (size_t i = 0; i < num_blocks_needed; i++) {
        needed_block_ptrs[i] = 0;
      }
            
      get_block_ptrs(fs, &file_inode, needed_block_ptrs, pos_in_file, num_blocks_needed);

      ssize_t num_written = 0;

      //go throught needed blocks, getting data from src
      for (size_t i = 0; i < num_blocks_needed; i++) {
        if (!needed_block_ptrs[i]) {
          break;
        }
        else if (i == 0) {
          //dont do full write -> i need to write func for these
          data_block_t temp_block;
          if (block_store_read(fs->bs, needed_block_ptrs[i], &temp_block)) {
            memcpy(INCREMENT_VOID(&temp_block, block_offset), INCREMENT_VOID(src, 0), block_bytes_left);
            if (!block_store_write(fs->bs, needed_block_ptrs[i], &temp_block)) {
              //return -3;
              break;
            }
            else {
              num_written = num_written + block_bytes_left;
            }
          }
          else {
            break;
          }
        } 
        else if (i == (num_blocks_needed - 1) && extra_block_bytes) {
          data_block_t temp_block;
          if (block_store_read(fs->bs, needed_block_ptrs[i], &temp_block)) {
            memcpy(INCREMENT_VOID(&temp_block, 0), INCREMENT_VOID(src, num_written), extra_block_bytes);
            if (!block_store_write(fs->bs, needed_block_ptrs[i], &temp_block)) {
              //return -3;
              break;
            }
            else {
              num_written = num_written + extra_block_bytes;
            }
          }
          else {
            break;
          }
        } 
        else {
          if (full_write(fs, INCREMENT_VOID(src, num_written), needed_block_ptrs[i])){
            num_written = num_written + BLOCK_SIZE;
          }
          else {
            break;
          }
        }
      }

      fs->fd_table.fd_pos[fd] = fs->fd_table.fd_pos[fd] + num_written;//update offset in fd table
      
      if (fs->fd_table.fd_pos[fd] > file_inode.mdata.size) {
        file_inode.mdata.size = fs->fd_table.fd_pos[fd];//update size
      }
      if (write_inode(fs, &file_inode, file_inode_ptr)) {
        return num_written;
      } 
    } 
  }
  return -1;
}

///
/// Reads data from the file linked to the given descriptor
///   Reading past EOF returns data up to EOF
///   R/W position in incremented by the number of bytes read
/// \param fs The F16FS containing the file
/// \param fd The file to read from
/// \param dst The buffer to write to
/// \param nbyte The number of bytes to read
/// \return number of bytes read (< nbyte IFF read passes EOF), < 0 on error
///
ssize_t fs_read(F16FS_t *fs, int fd, void *dst, size_t nbyte) {
  //basically the same as write
  if (fs == NULL || fd < 0 || fd >= DESCRIPTOR_MAX || !bitmap_test(fs->fd_table.fd_status, fd) || dst == NULL) {
    return -1;
  }
  else if (nbyte == 0) {
    return 0;
  }
  else {
    inode_t file_inode;
        
    inode_ptr_t file_inode_ptr = fs->fd_table.fd_inode[fd];//get inode from fd table
        
    if (!read_inode(fs, &file_inode, file_inode_ptr)) {
      return -2;
    }
    else {
      size_t pos_in_file = fs->fd_table.fd_pos[fd]; //pos to begin writing in file
      /*if (pos_in_file % BLOCK_SIZE == 0) {
        pos_in_file = 0;
      }*/
      pos_in_file = (pos_in_file % BLOCK_SIZE);

      size_t block_offset = POSITION_TO_INNER_OFFSET(pos_in_file);
      size_t block_bytes_left = BLOCK_SIZE - block_offset;
      size_t full_blocks;
      size_t extra_block_bytes;
      
      size_t byte_total = nbyte;
      if (pos_in_file + nbyte > file_inode.mdata.size) {
        byte_total = (file_inode.mdata.size - pos_in_file);//restrict byte total
      }

      if (byte_total > block_bytes_left) {//must do some partial reading in the file
        full_blocks = (byte_total - block_bytes_left) / BLOCK_SIZE;
        extra_block_bytes = (byte_total - block_bytes_left) % BLOCK_SIZE;
      } 
      else {//we can read fully
        block_bytes_left = byte_total;
        full_blocks = 0;
        extra_block_bytes = 0;
      }

      size_t blocks_to_read = (1 + full_blocks);
      if (extra_block_bytes) {
        blocks_to_read++;//find num of blocks to read
      }

      block_ptr_t needed_block_ptrs[blocks_to_read];
      for (size_t i = 0; i < blocks_to_read; i++) {
        needed_block_ptrs[i] = 0;
      }
            
      get_block_ptrs(fs, &file_inode, needed_block_ptrs, pos_in_file, blocks_to_read);

      ssize_t bytes_read = 0;

      //loop through all the blocks and copy data to the buffer
      for (size_t i = 0; i < blocks_to_read; i++) {
        if (!needed_block_ptrs[i]) {
          break;
        }
        else {
          data_block_t temp_block;
          if (block_store_read(fs->bs, needed_block_ptrs[i], &temp_block)) {
            memcpy(INCREMENT_VOID(dst, 0), INCREMENT_VOID(&temp_block, block_offset), block_bytes_left);
            bytes_read = bytes_read + block_bytes_left;
          }
          else {
            break;
          }
        } 
        if (i == (blocks_to_read - 1) && extra_block_bytes) {
          data_block_t temp_block;
          if (block_store_read(fs->bs, needed_block_ptrs[i], &temp_block)) {
            memcpy(INCREMENT_VOID(dst, bytes_read), INCREMENT_VOID(&temp_block, block_offset), block_bytes_left);
            bytes_read = bytes_read + block_bytes_left;
          }
          else {
            break;
          }
        } 
        else {
          if (full_read(fs, INCREMENT_VOID(dst, bytes_read), needed_block_ptrs[i])) {
            bytes_read = bytes_read + BLOCK_SIZE;
          }
          else {
            break;
          }
          bytes_read = bytes_read + BLOCK_SIZE;
        }
      }
      
      fs->fd_table.fd_pos[fd] = fs->fd_table.fd_pos[fd] + bytes_read;//update offset in fd table

      return bytes_read;
    }
  }
  return -1;
}

///
/// Deletes the specified file
///   Directories can only be removed when empty
///   Using a descriptor to a file that was deleted is undefined
/// \param fs The F16FS containing the file
/// \param path Absolute path to file to remove
/// \return 0 on success, < 0 on error
///
int fs_remove(F16FS_t *fs, const char *path) {
  if (fs == NULL || path == NULL) {
    return -1;
  }
  else {
    result_t file_status;
        
    locate_file(fs, path, &file_status);//find file

    if (file_status.success && file_status.found && file_status.inode) {
            
      inode_t file_inode; 
      inode_t file_parent_inode;
            
      if (read_inode(fs, &file_inode, file_status.inode)
      && read_inode(fs, &file_parent_inode, file_status.parent)) {
               
        dir_block_t curr_dir;
        size_t file_blocks;
                
        if (file_status.type == FS_REGULAR) {
          file_blocks = file_inode.mdata.size / BLOCK_SIZE;
                        
          if (file_inode.mdata.size % BLOCK_SIZE != 0) {
            file_blocks++;
          }
          for (size_t i = 0; i < DESCRIPTOR_MAX; i++) { 
            if (bitmap_test(fs->fd_table.fd_status, i)) {
              if (fs->fd_table.fd_inode[i] == file_status.inode) {
                fs_close(fs, i);//it is in fd table so get rid of it
              }
            }
          }
        }
        else if (file_status.type == FS_DIRECTORY) {
          file_blocks = 1;
          if (!full_read(fs, &curr_dir, file_status.block)) {
            return -1;//dir not empty
          } 
          if (curr_dir.mdata.size != 0) {
            return -1;//dir not empty
          }
        }
                
        if (file_blocks) {
          block_ptr_t block_ptrs[file_blocks];
                    
          get_block_ptrs(fs, &file_inode, block_ptrs, 0, file_blocks);
                    
          for (size_t i = 0; i < file_blocks; i++) {
            if (block_ptrs[i]) {
              block_store_release(fs->bs, block_ptrs[i]);//realease direct blocks
            }
          }

          if (file_inode.data_ptrs[6]) {
            block_store_release(fs->bs, file_inode.data_ptrs[6]);//release indirect blocks
          }
                    
          if (file_inode.data_ptrs[7]) {
            block_ptr_t indirect_block[INDIRECT_TOTAL];
                        
            if (!full_read(fs, &indirect_block, file_inode.data_ptrs[7])) {
              return -1;
            }
            
            bool all_removed = false;
            
            for (size_t i = 0; i < INDIRECT_TOTAL; i++) {
              if (!all_removed) {
                if (indirect_block[i]) {
                  block_store_release(fs->bs, indirect_block[i]);//check double indirect
                } 
                else {
                  all_removed = true;
                }
              }
            }
          }
        }
                
        memset(&file_inode, 0, sizeof(inode_t));//clear file inode

        dir_block_t curr_parent_dir;
                
        if (write_inode(fs, &file_inode, file_status.inode)) {
          if (full_read(fs, &curr_parent_dir, file_parent_inode.data_ptrs[0])) {
            for (size_t i = 0; i < DIR_REC_MAX; i++) {
              if (file_status.inode == curr_parent_dir.entries[i].inode) {
                curr_parent_dir.entries[i].fname[0] = '\0';
                curr_parent_dir.entries[i].inode = 0;
                curr_parent_dir.mdata.size--;//remove dir_entry
              }
            }
            if (full_write(fs, &curr_parent_dir, file_parent_inode.data_ptrs[0])) {
              return 0;
            }
          } 
        } 
      } 
    } 
  } 
  return -1;
}