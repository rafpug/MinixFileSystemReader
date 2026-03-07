#include <stdio.h>
#include "partition-reader.h"

void read_partition_table(FILE *fp, long base, 
                            struct part_entry table[MAX_PARTS]) {
    uint16_t signature;
    size_t ret;

    if (fseek(fp, base + VALID_TABLE_LOC, SEEK_SET)) {
        perror("Failed valid boot seek");
        exit(1);
    }

    ret = fread(&signature, sizeof(signature), 1, fp);
    if (ret != 1) {
        perror("Failed valid boot read");
        exit(1);
    }
    
    if (signature != VALID_TABLE_SIG) {
        perror("Invalid partition table");
        exit(1);
    }

    if (fseek(fp, base + PART_TABLE_LOC, SEEK_SET)) {
        perror("Failed partition table seek");
        exit(1);
    }
    
    ret = fread(table, sizeof(struct part_entry), MAX_PARTS, fp);
    if (ret != MAX_PARTS) {
        perror("Failed partition table read");
        exit(1);
    }

    if (table.type != MINIX_TYPE) {
        perror("Not a MINIX partition");
        exit(1);
    }
}
    
void read_superblock(FILE *fp, long base, struct superblock sb) {
    int ret;
    size_t nread;

    ret = fseek(fp, base + SUP_BLOCK_OFFSET, SEEK_SET);
    if (ret) {
        perror("Failed superblock seek");
        exit(1);
    }
    
    nread = fread(sb, sizeof(struct superblock), 1, fp);
    if (nread != 1) {
        perror("Failed superblock read");
        exit(1);
    }

    if (sb.magic != MINIX_MAGIC) {
        perror("Not a MINIX filesystem");
        exit(1);
    }
}



