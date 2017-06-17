#ifndef BLOCK_STORE_H__
#define BLOCK_STORE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Back store object
// It's an opaque object whose implementation is up to you
// (and implementation DOES NOT go here)
typedef struct block_store block_store_t;

///
/// Creates a new block_store file at the specified location
///  and returns a block_store object linked to it
/// \param fname the file to create
/// \return a pointer to the new object, NULL on error
///
block_store_t *block_store_create(const char *const fname);

///
/// Opens the specified block_store file
///  and returns a block_store object linked to it
/// \param fname the file to open
/// \return a pointer to the new object, NULL on error
///
block_store_t *block_store_open(const char *const fname);

///
/// Closes and frees a block_store object
/// \param bs block_store to close
///
void block_store_close(block_store_t *const bs);

///
/// Allocates a block of storage in the block_store
/// \param bs the block_store to allocate from
/// \return id of the allocated block, 0 on error
///
unsigned block_store_allocate(block_store_t *const bs);

///
/// Requests the allocation of a specified block id
/// \param bs block_store to allocate from
/// \param block_id block to attempt to allocate
/// \return bool indicating allocation success
///
bool block_store_request(block_store_t *const bs, const unsigned block_id);

///
/// Releases the specified block id so it may be used later
/// \param bs block_store object
/// \param block_id block to release
///
void block_store_release(block_store_t *const bs, const unsigned block_id);

///
/// Reads data from the specified block to the given data buffer
/// \param bs the object to read from
/// \param block_id the block to read from
/// \param dst the buffer to write to
/// \return bool indicating success
///
bool block_store_read(block_store_t *const bs, const unsigned block_id, void *const dst);

///
/// Writes data from the given buffer to the specified block
/// \param bs the object to write to
/// \param block_id the block to write to
/// \param src the buffer to read from
/// \return bool indicating success
///
bool block_store_write(block_store_t *const bs, const unsigned block_id, const void *const src);

#ifdef __cplusplus
}
#endif
#endif