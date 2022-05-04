#ifndef _SYS_ARCH_H_
#define _SYS_ARCH_H_

/**
 * @brief word size = 1 in wramp architecture
 * this means sizeof(unsigned int) == sizeof(char) == 1
 * whereas sizeof(unsigned int) == 4, sizeof(char) == 1 in other architectures
 * This has implications on disk.c, where it is pre-compiled on other architectures
 * in fs/fsutil/fsutil.c, but used in wramp architecture.
 * Thus sizing conversion are required
 * 
 */

#ifdef __wramp__
#define CHAR_SIZE   4
#else
#define CHAR_SIZE   1
#endif

#define DWORD_SIZE                       (sizeof(unsigned int))
#define TO_DWORD_SIZE(x)                 (x / DWORD_SIZE)
#define TO_CHAR_SIZE_FROM_WRAMP(x)      (x * DWORD_SIZE)
#define ARCH_CHAR_SIZE(x)         (x /= CHAR_SIZE)
#define DEARCH_CHAR_SIZE(x)       (x *= CHAR_SIZE)

#endif
