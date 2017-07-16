#ifndef SYSFS_H
#define SYSFS_H

#ifndef O_BINARY
#define O_BINARY    0x00
#endif

#ifndef O_RDONLY
#define O_RDONLY	0x00
#endif

#ifndef O_WRONLY
#define O_WRONLY	0x01
#endif

#ifndef O_RDWR
#define O_RDWR		0x02
#endif

#ifndef O_CREAT		
#define O_CREAT 	0x0100
#endif

#ifndef O_EXCL
#define O_EXCL		0x0200
#endif

#ifndef O_TRUNC
#define O_TRUNC		0x01000
#endif

#ifndef O_APPEND
#define O_APPEND	0x02000
#endif

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif

#ifndef SEEK_END
#define SEEK_END	2
#endif

#ifndef EBUSY
#define EBUSY	16
#endif

#ifndef ENODEV
#define ENODEV	19
#endif

#ifndef EINVAL
#define EINVAL	22
#endif

#ifndef EBADF
#define EBADF	9
#endif

#ifndef EACCESS
#define EACCESS	13
#endif

#ifndef EXDEV	
#define EXDEV	18
#endif

#ifndef ENOENT
#define ENOENT	2
#endif

#ifndef ENOSPC
#define ENOSPC	28
#endif

#ifndef ENOTEMPTY
#define ENOTEMPTY 39
#endif

#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EEXIST
#define EEXIST 17
#endif

#ifndef ENOTDIR
#define ENOTDIR 20
#endif

#ifndef EISDIR
#define EISDIR 21
#endif


// Mode flags

#ifndef S_IFMT
#define S_IFMT		0170000
#endif

#ifndef S_IFLNK
#define S_IFLNK		0120000
#endif

#ifndef S_IFDIR
#define S_IFDIR		0040000
#endif

#ifndef S_IFREG
#define S_IFREG		0100000
#endif

#ifndef S_IREAD 
#define S_IREAD		0000400
#endif

#ifndef S_IWRITE
#define	S_IWRITE	0000200
#endif

typedef int DIR;
typedef unsigned int mode_t;
typedef int direct;
typedef int stat;
typedef int off_t;

int open(const char* pathname, int flags, int mode);

int close(int fd);

int unlink(const char *pathname);

int read(int fd, void *buf, unsigned int nbyte);

int write(int fd, const void *buf, unsigned int nbyte);

DIR* opendir(const char *pathname);

int closedir(DIR* dp);

int mkdir(const char *pathname, mode_t mode);

int rmdir(const char *pathname);

direct *yaffs_readdir(DIR *dp);

int fstat(int fd, stat *sbuf);

off_t lseek(int fd, off_t offset, int whence);

int chmod(const char *path, mode_t mode);

#endif
