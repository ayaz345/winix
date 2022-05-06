#ifndef _UNISTD_H_
#define _UNISTD_H_ 1

#ifndef _POSIX_SOURCE
#define _POSIX_ 1
#endif

#include <sys/ioctl.h>
#include <stddef.h>
#include <sys/ipc.h>
#include <sys/unistd.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include <errno.h>
#include <signal.h>

int isatty(int fd);
void sync();
int getdents( int fd, struct dirent *dirp, unsigned int count);

#if defined(__wramp__) & !defined(_SYSTEM)
#define sync()                              wramp_syscall(SYNC)
#endif

#endif
