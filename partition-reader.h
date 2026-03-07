/* This header provides the structs for used blocks and tables
 * It also provides some abstractions for finding the MINIX
 * file system in the image */

#include <stdint.h>
#include <stdio.h>

#define SECTORE_SIZE 512

#define DIRECT_ZONES 7

#define PART_TABLE_LOC 0x1BE
#define MINIX_PART_TYPE 0x81
#define VALID_BOOT_510 0x55
#define VALID_BOOT_511 0xAA
#define MINIX_MAGIC 0x4D5A
#define MINIX_MAGIC_REVERSE 0x5A4D
#define INODE_SIZE 64
#define DIR_ENTRY_SIZE 64
#define SUP_BLOCK_OFFSET 1024

struct __attribute__ ((__packed__)) part_table {
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

struct __attribute__ ((__packed__)) super_block {
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
