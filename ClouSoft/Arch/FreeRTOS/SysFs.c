#include "SysFs.h"

int open(const char* pathname, int flags, int mode)
{
    return 0;
}

int close(int fd)
{
    return 0;
}

int unlink(const char *pathname)
{
	return 0;
}

int read(int fd, void *buf, unsigned int nbyte)
{
	return 0;
}

int write(int fd, const void *buf, unsigned int nbyte)
{
	return 0;
}

DIR* opendir(const char *pathname)
{
    return 0;
}

int closedir(DIR* dp)
{
    return 0;
}

int mkdir(const char *pathname, mode_t mode)
{
	return 0;
}

int rmdir(const char *pathname) 
{
	return 0;
}

direct *yaffs_readdir(DIR *dp)
{
     return 0;
}

int fstat(int fd, stat *sbuf)
{
    return 0;
}

off_t lseek(int fd, off_t offset, int whence) 
{
    return 0;
}

int chmod(const char *path, mode_t mode)
{
	return 0;
}
