#include <stdio.h>
#include "partition-reader.h"

void read_partition_table (FILE *fp, long base, 
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
}
    

