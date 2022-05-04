#ifndef _FS_INODE_H_
#define _FS_INODE_H_ 1

#include <uchar.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <stddef.h>
#include <fs/type.h>
#include <fs/common.h>
#include <winix/type.h>
#include <winix/comm.h>


typedef struct inode {
    mode_t i_mode;        /* file type, protection, etc. */ //8 bytes
    nlink_t i_nlinks;        /* how many links to this file */ //8bytes
    uid_t i_uid;            /* user id of the file's owner */ //8bytes
    gid_t i_gid;            /* group number */ //8 bytes
    off_t i_size;            /* current file size in bytes */ //8 bytes
    time_t i_atime;        /* time of last access (V2 only) */ //8
    time_t i_mtime;        /* when was file data last changed */ //8
    time_t i_ctime;        /* when was inode itself changed (V2 only)*/ //8
    zone_t i_zone[NR_TZONES]; /* zone numbers for data blocks */

    struct device* i_dev;            /* which device is the inode on */
    ino_t i_num;            /* inode number on its (minor) device */
    int i_count;            /* # times inode used; 0 means slot is free */
    block_t i_ndblock;        /* # direct block, where the inode info is stored in the inode table */
    struct superblock *i_sb;    /* pointer to super block for inode's device */
    unsigned int i_flags;
    struct list_head pipe_reading_list;
    struct list_head pipe_writing_list;

    // char i_dirt;            /* CLEAN or DIRTY */
    // char i_pipe;            /* set to I_PIPE if pipe */
    // char i_mount;            /* this bit is set if file mounted on */
    // char i_seek;            /* set on LSEEK, cleared on READ/WRITE */
    // char i_update;        /* the ATIME, CTIME, and MTIME bits are here */
} inode_t;

struct inode_disk {
    mode_t i_mode;        /* file type, protection, etc. */ //8 bytes
    nlink_t i_nlinks;        /* how many links to this file */ //8bytes
    uid_t i_uid;            /* user id of the file's owner */ //8bytes
    gid_t i_gid;            /* group number */ //8 bytes
    off_t i_size;            /* current file size in bytes */ //8 bytes
    time_t i_atime;        /* time of last access (V2 only) */ //8
    time_t i_mtime;        /* when was file data last changed */ //8
    time_t i_ctime;         /* when was inode itself changed (V2 only)*/ //8
    zone_t i_zone[NR_TZONES]; /* zone numbers for data blocks */
};

struct zone_iterator{
    struct inode* i_inode;
    block_t i_zone_idx;
};

struct dirent_iterator{
    struct winix_dirent* dirent;
    struct winix_dirent* dirent_end;
    struct block_buffer* buffer;
    struct zone_iterator zone_iter;
};

#define INODE_DISK_SIZE (sizeof(struct inode_disk))


extern inode_t inode_table[NR_INODES];

#define MAX_INDIRECT_NR_ZONE    2
#define MAX_ZONES               (NR_TZONES * MAX_INDIRECT_NR_ZONE + NR_TZONES - MAX_INDIRECT_NR_ZONE)
#define NR_DIRECT_ZONE          (NR_TZONES - MAX_INDIRECT_NR_ZONE)

#define NIL_INODE (inode_t *) 0    /* indicates absence of inode slot */
#define INODE_FLAG_DIR          0x00000001
#define INODE_FLAG_PIPE         0x00000002
#define INODE_FLAG_MOUNT        0x00000004
#define INODE_FLAG_SEEK         0x00000008
#define INODE_FLAG_MEM_DIR      0x00000010      // temp dir like /dev
#define INODE_FLAG_ZONE         0x00000020      // inode for indirect zone
#define INODE_FLAG_DIRTY        0x00000040      

#endif

