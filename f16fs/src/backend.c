#include "backend.h"

#include <string.h>
#include <time.h>

bool read_inode(const F16FS_t *fs, void *data, const inode_ptr_t inode_number) {
    if (fs && data) {
        inode_t buffer[INODES_PER_BOCK];
        if (block_store_read(fs->bs, INODE_TO_BLOCK(inode_number), buffer)) {
            memcpy(data, &buffer[INODE_INNER_IDX(inode_number)], sizeof(inode_t));
            return true;
        }
    }
    return false;
}

bool write_inode(F16FS_t *fs, const void *data, const inode_ptr_t inode_number) {
    if (fs && data) {  // checking if the inode number is valid is a tautology :/
        inode_t buffer[INODES_PER_BOCK];
        if (block_store_read(fs->bs, INODE_TO_BLOCK(inode_number), buffer)) {
            memcpy(&buffer[INODE_INNER_IDX(inode_number)], data, sizeof(inode_t));
            return block_store_write(fs->bs, INODE_TO_BLOCK(inode_number), buffer);
        }
    }
    return false;
}

// might as well make versions that do whole blocks. Better encapsulation?
// All calls are verified a bit more before happening, which is good.
bool full_read(const F16FS_t *fs, void *data, const block_ptr_t block) {
    if (fs && data) {  // you can read from the inode table...
        return block_store_read(fs->bs, block, data);
    }
    return false;
}

bool full_write(F16FS_t *fs, const void *data, const block_ptr_t block) {
    if (fs && data && block >= DATA_BLOCK_OFFSET) {  // but you can't write to it. Not in bulk.
                                                     // there is NO reason to do a bulk write to the inode table
        return block_store_write(fs->bs, block, data);
    }
    return false;
}

F16FS_t *ready_file(const char *path, const bool format) {
    F16FS_t *fs = (F16FS_t *) malloc(sizeof(F16FS_t));
    if (fs) {
        if (format) {
            // get inode table
            // format root
            // That's it?

            // oh, also, ya know, make the back store object. oops.
            fs->bs = block_store_create(path);
            if (fs->bs) {
                bool valid = true;
                // + 1 to snag the root dir block because lazy
                for (int i = INODE_BLOCK_OFFSET; i < (DATA_BLOCK_OFFSET + 1) && valid; ++i) {
                    valid &= block_store_request(fs->bs, i);
                }
                // inode table is already blanked because back_store blanks all data (woo)
                if (valid) {
                    // I'm actually not sure how to do this
                    // It's going to look like a mess
                    uint32_t right_now = time(NULL);
                    inode_t root_inode = {{0, 0777, right_now, right_now, right_now, 0, FS_DIRECTORY, 1, {0}},
                                          {DATA_BLOCK_OFFSET, 0, 0, 0, 0, 0, 0, 0}};
                    // fname technically invalid, but it's root so deal
                    // mdata actually might not be used in a dir record. Idk.
                    // block pointer set, rest are invalid
                    // break point HERE to make sure that constructed right
                    valid &= write_inode(fs, &root_inode, 0);
                }
                if (!valid) {
                    // weeeeeeeh
                    block_store_close(fs->bs);
                    fs->bs = NULL;  // just set it and trigger the free elsewhere
                }
            }
        } else {
            fs->bs = block_store_open(path);
            // ... that's it?
        }
        if (fs->bs) {
            fs->fd_table.fd_status = bitmap_create(DESCRIPTOR_MAX);
            // Eh, won't bother blanking out tables, since that's the point of the bitmap
            if (fs->fd_table.fd_status) {
                return fs;
            }
        }
        free(fs);
    }
    return NULL;
}

/*
hunts down the requested file, if it exists, filling out all sorts of little bits of data
typedef struct {
    bool success; - Did the operation complete? (generally just parameter issues)
    bool found; - Did we find the file? Does it exist?
    bool valid; - N/A
    inode_ptr_t inode; - IF FOUND: inode of file
    inode_ptr_t parent; - IF FOUND: inode of parent directory of file
    block_ptr_t block; - IF FOUND AND TYPE DIRECTORY: Dir block for directory
    file_t type; - IF FOUND: Type of file
    uint64_t total; - N/A
    void *data; - Pointer to last token parsed (in given path string)
                    Either the filename, or the token we died on
} result_t;
*/
void locate_file(const F16FS_t *const fs, const char *abs_path, result_t *res) {
    if (res) {
        memset(res, 0x00, sizeof(result_t));  // IMMEDIATELY blank it
        if (fs && abs_path) {
            // need to get the token processing loop started (oh boy!)
            const size_t path_len = strnlen(abs_path, FS_PATH_MAX);
            if (path_len != 0 && abs_path[0] == '/' && path_len < FS_PATH_MAX) {
                // ok, path is something we should at least bother trying to look at
                char *path_copy = strndup(abs_path, path_len);
                if (path_copy) {
                    result_t scan_results = {true, true, false, 0, 0, 0, 0, 0, 0, NULL};  // hey, cool, I found root!
                    const char *delims    = "/";
                    res->success          = true;
                    res->found            = true;  // I'm going to assume it all works out, don't go making me a liar
                    res->inode            = 0;
                    res->block            = ROOT_DIR_BLOCK;
                    res->type             = FS_DIRECTORY;
                    res->data             = (void *) abs_path;
                    char *token           = strtok(path_copy, delims);
                    // Hardcoding results for root in case there aren't any tokens (path was "/")
                    while (token) {
                        // update the dir pointer
                        res->data = (void *) (abs_path + (token - path_copy));
                        // Cool. Does the next token exist in the current directory?
                        scan_directory(fs, token, scan_results.inode, &scan_results);
                        if (scan_results.success && scan_results.found) {
                            // Good. It existed. Cycle.
                            res->parent = scan_results.parent;
                            res->inode  = scan_results.inode;
                            token       = strtok(NULL, delims);
                            continue;
                        }
                        // welp. Something's broken. File not found.
                        res->found = false;
                        break;
                    }
                    if (res->found) {
                        inode_t found_file;
                        if (read_inode(fs, &found_file, res->inode)) {
                            res->type = found_file.mdata.type;
                            if (res->type == FS_DIRECTORY) {
                                res->block = found_file.data_ptrs[0];
                            }
                            free(path_copy);
                            return;
                        }
                        // block_store ate it I guess? That's bad.
                        // What do we do now? (die.)
                        memset(res, 0x00, sizeof(result_t));  // all that work for nothing
                    }
                    free(path_copy);
                }
            }
        }
    }
}
/*
Flips through the specified directory, finding the specified file (hopefully)
typedef struct {
    bool success; - Did the operation complete? (generally just parameter issues) Was it a dir?
    bool found; - IF VALID: Did we find the file? Does it exist?
    bool valid; - Was the filename valid?
    inode_ptr_t inode; - IF FOUND: inode of file
    inode_ptr_t parent; - IF SUCCESS: Literally the inode number you fed us
    block_ptr_t block; - IF SUCCESS: Data block of directory.
                            Shouldn't never need it, but we know it, so we'll share
    file_t type; - IF FOUND: type of the file found
    uint64_t total; - IF SUCCESS: Number of files in the given directory
    uint64_t pos; - IF FOUND: Entry position in directory
    void *data; - N/A
} result_t;
*/
void scan_directory(const F16FS_t *const fs, const char *fname, const inode_ptr_t inode, result_t *res) {
    if (res) {
        memset(res, 0x00, sizeof(result_t));
        if (fs && fname) {
            // inode number is always valid - tbh, that may mask errors and could be considered bad
            inode_t dir_inode;
            dir_block_t dir_data;
            if (read_inode(fs, &dir_inode, inode) && INODE_IS_TYPE(&dir_inode, FS_DIRECTORY)
                && full_read(fs, &dir_data, dir_inode.data_ptrs[0])) {
                res->success = true;
                res->block   = dir_inode.data_ptrs[0];
                res->total   = dir_data.mdata.size;
                res->parent  = inode;
                // let's validate the fname
                const size_t fname_len = strnlen(fname, FS_FNAME_MAX);
                if (fname_len != 0 && fname_len < FS_FNAME_MAX) {
                    // Alrighty, we got the inode and block read in.
                    // fname is vaguely validated
                    res->valid = true;
                    for (unsigned i = 0; i < DIR_REC_MAX; ++i) {
                        if (strncmp(fname, dir_data.entries[i].fname, FS_FNAME_MAX) == 0) {
                            // found it!
                            res->found = true;
                            res->inode = dir_data.entries[i].inode;
                            res->pos   = i;
                            return;
                        }
                    }
                }
            }
        }
    }
}

// Just what it sounds like. 0 on error
inode_ptr_t find_free_inode(const F16FS_t *const fs) {
    if (fs) {
        inode_t inode_block[INODES_PER_BOCK];
        inode_ptr_t free_inode = 0;
        for (unsigned blk = INODE_BLOCK_OFFSET; blk < DATA_BLOCK_OFFSET; ++blk) {
            if (full_read(fs, &inode_block, blk)) {
                for (unsigned i = 0; i < INODES_PER_BOCK; ++i, ++free_inode) {
                    if (! inode_block[i].mdata.in_use) {
                        return free_inode;
                        // potentially a conversion warning because integer truncation/depromotion
                    }
                }
            } else {
                return 0;  // :/
            }
        }
    }
    return 0;
}

// Fills array with data blocks
void get_block_ptrs(F16FS_t *fs, inode_t *file_inode, block_ptr_t *block_ptrs, size_t pos, size_t num_of_blocks) {
  if (fs == NULL || file_inode == NULL || block_ptrs == NULL || num_of_blocks == 0) {
    return;
  }
  else {
    size_t block_index = POSITION_TO_BLOCK_INDEX(pos);
    
    size_t fbi = block_index; //file block ind
    size_t bpi = 0; //block ptr ind
    
    bool progress = true;

    //get direct block ptrs
    while (fbi < DIRECT_TOTAL && bpi < num_of_blocks) {
      if (!progress) {
        break;
      }
      if (!file_inode->data_ptrs[fbi]) {//not currently a block at this ind
      
        file_inode->data_ptrs[fbi] = block_store_allocate(fs->bs);
        
        if (!file_inode->data_ptrs[fbi]) {
          progress = false;
        }
      }
      if (progress) {
        block_ptrs[bpi] = file_inode->data_ptrs[fbi];//got the block
        bpi++;
        fbi++;
      }
    }

    //get indirect block ptrs
    if (fbi < (DIRECT_TOTAL + INDIRECT_TOTAL) && bpi < num_of_blocks) {
      if (progress) {
        block_ptr_t indirect_block[INDIRECT_TOTAL] = {0};
          if (!(file_inode->data_ptrs[6])) {//need new indirect block
        
            file_inode->data_ptrs[6] = block_store_allocate(fs->bs);
          
            if (!(file_inode->data_ptrs[6])) {
              progress = false;
            }
          } 
          else {
            if (!full_read(fs, indirect_block, file_inode->data_ptrs[6])) {//get curr indirect block
              progress = false;
            }
          }
          if (progress) {//get all other blocks from indirect block
            for (size_t i = (fbi-DIRECT_TOTAL) % INDIRECT_TOTAL; i < INDIRECT_TOTAL && bpi < num_of_blocks; i++) {
              if(!progress) {
                break;
              }
              if (!indirect_block[i]) {
                indirect_block[i] = block_store_allocate(fs->bs);
                if (!indirect_block[i]) {
                  progress = false;
                }
              }
              if (progress) {
                block_ptrs[bpi] = indirect_block[i];
                bpi++;
                fbi++;
              }
            }
            if (!full_write(fs, indirect_block, file_inode->data_ptrs[6])) {
              progress = false;
            }
          }
        }
      }

    //get double indirect block ptrs 
    if (bpi < num_of_blocks) {
      if (progress) {
        block_ptr_t db_ind_block[INDIRECT_TOTAL] = {0};
        if (!(file_inode->data_ptrs[7])) {
          file_inode->data_ptrs[7] = block_store_allocate(fs->bs);
          if (!(file_inode->data_ptrs[7])) {
            progress = false;
          }
        } 
        else {
          if(!full_read(fs, db_ind_block, file_inode->data_ptrs[7])) {
            progress = false;
          }
        }
        if (progress) {
          for (size_t j = (fbi-(DIRECT_TOTAL + INDIRECT_TOTAL)) / INDIRECT_TOTAL; bpi < num_of_blocks && progress && j < INDIRECT_TOTAL; j++) {
            block_ptr_t indirect_block[INDIRECT_TOTAL] = {0};
            if (!db_ind_block[j]) {
              db_ind_block[j] = block_store_allocate(fs->bs);
              if (!db_ind_block[j]) {
                progress = false;
              }
            } 
            else {
              if (!full_read(fs, indirect_block, db_ind_block[j])) {
                progress = false;
              }
            }
            if (progress) {
              for (size_t k = (fbi-(DIRECT_TOTAL + INDIRECT_TOTAL)) % INDIRECT_TOTAL; bpi < num_of_blocks && progress && k < INDIRECT_TOTAL; k++) {
                if (!indirect_block[k]) {
                  indirect_block[k] = block_store_allocate(fs->bs);
                  if(!indirect_block[k]) {
                    progress = false;
                  }
                }
                if (progress) {
                  block_ptrs[bpi] = indirect_block[k];
                  bpi++;
                  fbi++;
                }
              }
              if (!full_write(fs, indirect_block, db_ind_block[j])) {
                progress = false;
              }
            }
          }
        }
        if (!full_write(fs, db_ind_block, file_inode->data_ptrs[7])) {
            progress = false;
        }
      }
    }
  }
  return;
}