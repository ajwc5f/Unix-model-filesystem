#ifndef F16FS_H__
#define F16FS_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include <dyn_array.h>
typedef struct F16FS F16FS_t;
typedef enum { FS_SEEK_SET, FS_SEEK_CUR, FS_SEEK_END } seek_t;
typedef enum { FS_REGULAR, FS_DIRECTORY } file_t;
#define FS_FNAME_MAX (64)
// INCLUDING null terminator
typedef struct {
    // You can add more if you want
    // vvv just don't remove or rename these vvv
    char name[FS_FNAME_MAX];
    file_t type;
} file_record_t;
///
/// Formats (and mounts) an F16FS file for use
/// \param fname The file to format
/// \return Mounted F16FS object, NULL on error
///
F16FS_t *fs_format(const char *path);
///
/// Mounts an F16FS object and prepares it for use
/// \param fname The file to mount
/// \return Mounted F16FS object, NULL on error
///
F16FS_t *fs_mount(const char *path);
///
/// Unmounts the given object and frees all related resources
/// \param fs The F16FS object to unmount
/// \return 0 on success, < 0 on failure
///
int fs_unmount(F16FS_t *fs);
///
/// Creates a new file at the specified location
///   Directories along the path that do not exist are NOT created
/// \param fs The F16FS containing the file
/// \param path Absolute path to file to create
/// \param type Type of file to create (regular/directory)
/// \return 0 on success, < 0 on failure
///
int fs_create(F16FS_t *fs, const char *path, file_t type);
///
/// Opens the specified file for use
///   R/W position is set to the beginning of the file (BOF)
///   Directories cannot be opened
/// \param fs The F16FS containing the file
/// \param path path to the requested file
/// \return file descriptor to the requested file, < 0 on error
///
int fs_open(F16FS_t *fs, const char *path);
///
/// Closes the given file descriptor
/// \param fs The F16FS containing the file
/// \param fd The file to close
/// \return 0 on success, < 0 on failure
///
int fs_close(F16FS_t *fs, int fd);
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
off_t fs_seek(F16FS_t *fs, int fd, off_t offset, seek_t whence);
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
ssize_t fs_read(F16FS_t *fs, int fd, void *dst, size_t nbyte);
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
ssize_t fs_write(F16FS_t *fs, int fd, const void *src, size_t nbyte);
///
/// Deletes the specified file
///   Directories can only be removed when empty
///   Using a descriptor to a file that was deleted is undefined
/// \param fs The F16FS containing the file
/// \param path Absolute path to file to remove
/// \return 0 on success, < 0 on error
///
int fs_remove(F16FS_t *fs, const char *path);
///
/// Populates a dyn_array with information about the files in a directory
///   Array contains up to 7 file_record_t structures
/// \param fs The F16FS containing the file
/// \param path Absolute path to the directory to inspect
/// \return dyn_array of file records, NULL on error
///
dyn_array_t *fs_get_dir(F16FS_t *fs, const char *path);
///
/// !!! Graduate Level/Undergrad Bonus !!!
/// !!! Activate tests from the cmake !!!
///
/// Moves the file from one location to the other
///   Moving files does not affect open descriptors
/// \param fs The F16FS containing the file
/// \param src Absolute path of the file to move
/// \param dst Absolute path to move the file to
/// \return 0 on success, < 0 on error
///
int fs_move(F16FS_t *fs, const char *src, const char *dst);
///
/// !!! Graduate Level/Undergrad Bonus !!!
/// !!! Activate tests from the cmake !!!
///
/// Creates a hardlink at dst pointing to the file at src
/// \param fs The F16FS containing the file
/// \param src Absolute path of the file to link to
/// \param dst Absolute path to the link to create
/// \return 0 on success, < 0 on error
///
int fs_link(F16FS_t *fs, const char *src, const char *dst);
#ifdef __cplusplus
}
#endif
#endif