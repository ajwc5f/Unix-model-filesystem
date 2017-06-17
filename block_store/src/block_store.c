#include "block_store.h"

#include <bitmap.h>

#include <fcntl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK_COUNT 65536
#define BLOCK_SIZE 512
#define FBM_BLOCK_COUNT 16
#define BYTE_TOTAL ((BLOCK_COUNT) * (BLOCK_SIZE))
#define DATA_BLOCK_BYTE_TOTAL ((65536 - (FBM_BLOCK_COUNT)) * (BLOCK_SIZE))
#define FBM_BYTE_TOTAL ((BLOCK_SIZE) * (FBM_BLOCK_COUNT))
#define DATA_BLOCK_START (FBM_BLOCK_COUNT)


struct block_store {
    int fd;
    bitmap_t *fbm;
    uint8_t *data_blocks;
};

int create_file(const char *const fname) {
    if (fname) {
        int fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd != -1) {
            if (ftruncate(fd, BYTE_TOTAL) != -1) {
                return fd;
            }
            close(fd);
        }
    }
    return -1;
}
int check_file(const char *const fname) {
    if (fname) {
        int fd = open(fname, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd != -1) {
            struct stat file_info;
            if (fstat(fd, &file_info) != -1 && file_info.st_size == BYTE_TOTAL) {
                return fd;
            }
            close(fd);
        }
    }
    return -1;
}


block_store_t *block_store_init(const bool init, const char *const fname) {
    if (fname) {
        block_store_t *bs = (block_store_t *) malloc(sizeof(block_store_t));
        if (bs) {
            bs->fd = init ? create_file(fname) : check_file(fname);
            if (bs->fd != -1) {
                bs->data_blocks = (uint8_t *) mmap(NULL, BYTE_TOTAL, PROT_READ | PROT_WRITE, MAP_SHARED, bs->fd, 0);
                if (bs->data_blocks != (uint8_t *) MAP_FAILED) {
                    // Woo hoo! Done. Mostly. Kinda.
                    if (init) {
                        // init FBM and wipe remaining data
                        // Could/should be done in create_file
                        // but it's so much easier here...
                        memset(bs->data_blocks, 0xFF, FBM_BLOCK_COUNT >> 3);
                        memset(bs->data_blocks + FBM_BYTE_TOTAL, 0x00, DATA_BLOCK_BYTE_TOTAL);
                    }
                    // Not quite sure what to do with madvise
                    // Honestly, I feel like a split mapping may be best
                    // Sequential for the FBM, random for the data
                    // but I'll just not mess with it unless I get the time to profile them
                    // madvise()
                    bs->fbm = bitmap_overlay(BLOCK_COUNT, bs->data_blocks);
                    if (bs->fbm) {
                        return bs;
                    }
                    munmap(bs->data_blocks, BYTE_TOTAL);
                }
                close(bs->fd);
            }
            free(bs);
        }
    }
    return NULL;
}

block_store_t *block_store_create(const char *const fname) {
    return block_store_init(true, fname);
}

block_store_t *block_store_open(const char *const fname) {
    return block_store_init(false, fname);
}

void block_store_close(block_store_t *const bs) {
    if (bs) {
        bitmap_destroy(bs->fbm);
        munmap(bs->data_blocks, BYTE_TOTAL);
        close(bs->fd);
        free(bs);
    }
}

unsigned block_store_allocate(block_store_t *const bs) {
    if (bs) {
        size_t free_block = bitmap_ffz(bs->fbm);
        if (free_block != SIZE_MAX) {
            bitmap_set(bs->fbm, free_block);
            return free_block;
        }
    }
    return 0;
}

bool block_store_request(block_store_t *const bs, const unsigned block_id) {
    if (bs && block_id >= DATA_BLOCK_START && block_id <= BLOCK_COUNT) {
        if (!bitmap_test(bs->fbm, block_id)) {
            bitmap_set(bs->fbm, block_id);
            return true;
        }
    }
    return false;
}

void block_store_release(block_store_t *const bs, const unsigned block_id) {
    if (bs && block_id >= DATA_BLOCK_START && block_id <= BLOCK_COUNT) {
        bitmap_reset(bs->fbm, block_id);
    }
}

bool block_store_read(block_store_t *const bs, const unsigned block_id, void *const dst) {
    if (bs && dst && block_id >= DATA_BLOCK_START && block_id <= BLOCK_COUNT /* && bitmap_set(bs->fbm,block_id) */) {
        memcpy(dst, bs->data_blocks + (BLOCK_SIZE * block_id), BLOCK_SIZE);
        return true;
    }
    return false;
}


bool block_store_write(block_store_t *const bs, const unsigned block_id, const void *const src) {
    if (bs && src && block_id >= DATA_BLOCK_START && block_id <= BLOCK_COUNT /* && bitmap_set(bs->fbm,block_id) */) {
        memcpy(bs->data_blocks + (BLOCK_SIZE * block_id), src, BLOCK_SIZE);
        return true;
    }
    return false;
}
