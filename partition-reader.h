/* This header provides the structs for used blocks and tables
 * It also provides some abstractions for finding the MINIX
 * file system in the image */

#include <stdint.h>
#include <stdio.h>

#define SECTOR_SIZE 512
#define DIRECT_ZONES 7
#define MAX_PARTS 4
#define MAX_NAME 60
#define PERM_PRINT_SIZE 11

#define PART_TABLE_LOC 0x1BE
#define MINIX_TYPE 0x81
#define VALID_TABLE_LOC 510
#define VALID_TABLE_SIG 0xAA55
#define MINIX_MAGIC 0x4D5A
#define INODE_SIZE 64
#define DIR_ENTRY_SIZE 64
#define SUP_BLOCK_OFFSET 1024

#define TYPE_MASK 0170000
#define REG_MASK 0100000
#define DIR_MASK 0000400
#define OWNER_R_PERM 0000400
#define OWNER_W_PERM 0000200
#define OWNER_X_PERM 0000100
#define GROUP_R_PERM 0000040
#define GROUP_W_PERM 0000020
#define GROUP_X_PERM 0000010
#define OTHER_R_PERM 0000004
#define OTHER_W_PERM 0000002
#define OTHER_X_PERM 0000001

#define FOUND 1
#define VERBOSE 1
#define INVALID_PART -1
#define INVALID_SUB -1

struct __attribute__ ((__packed__)) part_entry {
    uint8_t bootind;
    uint8_t start_head;
    uint8_t start_sec;
    uint8_t start_cyl;
    uint8_t type;
    uint8_t end_head;
    uint8_t end_sec;
    uint8_t end_cyl;
    uint32_t lFirst;
    uint32_t size;
};

struct __attribute__ ((__packed__)) superblock {
    uint32_t ninodes;
    uint16_t pad1;
    int16_t i_blocks;
    int16_t z_blocks;
    uint16_t firstdata;
    int16_t log_zone_size;
    int16_t pad2;
    uint32_t max_file;
    uint32_t zones;
    int16_t magic;
    int16_t pad3;
    uint16_t blocksize;
    uint8_t subversion;
};

struct __attribute__ ((__packed__)) inode {
    uint16_t mode;
    uint16_t links;
    uint16_t uid;
    uint16_t gid;
    uint32_t size;
    int32_t atime;
    int32_t mtime;
    int32_t ctime;
    uint32_t zone[DIRECT_ZONES];
    uint32_t indirect;
    uint32_t two_indirect;
    uint32_t unused;
};

struct __attribute__ ((__packed__)) dir_entry {
    uint32_t inode;
    unsigned char name[MAX_NAME];
}


