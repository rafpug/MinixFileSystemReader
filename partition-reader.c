#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "partition-reader.h"

/* Modifies the given string to be the perms representing the given mode */
void stringify_perms(uint16_t mode, char perms[PERM_PRINT_SIZE]) {
    perms[PERM_PRINT_SIZE - 1] = '\0';

    uint16_t type = mode & TYPE_MASK;
    if (type == REG_MASK) {
        perms[0] = '-';
    }
    else if (type == DIR_MASK) {
        perms[0] = 'd';
    }
    else {
        perms[0] = '?';
    }
    
    perms[1] = (mode & OWNER_R_PERM) ? 'r' : '-';
    perms[2] = (mode & OWNER_W_PERM) ? 'w' : '-';
    perms[3] = (mode & OWNER_X_PERM) ? 'x' : '-';
    perms[4] = (mode & GROUP_R_PERM) ? 'r' : '-';
    perms[5] = (mode & GROUP_W_PERM) ? 'w' : '-';
    perms[6] = (mode & GROUP_X_PERM) ? 'x' : '-';
    perms[7] = (mode & OTHER_R_PERM) ? 'r' : '-';
    perms[8] = (mode & OTHER_W_PERM) ? 'w' : '-';
    perms[9] = (mode & OTHER_X_PERM) ? 'x' : '-'; 
}

/* Iterates through the zones of the given inodes, printing them to stderr */
void print_zones(struct inode inode) {
    int i;
    int length = (int) strlen("zone[_]    = "); 
    for (i=0; i < DIRECT_ZONES; i++) {
        /* Formats the print with padding appropriate padding*/
        fprintf(stderr, "%*szone[%d]    = %*u\n", ZONE_PRINT_PADDING, " ",
                   i, ZONE_PRINT_ALIGN - length, inode.zone[i]);
    }
} 

/* Prints all the fields/attributes of the given inode */
void print_inode(struct inode i) {
    char perms[PERM_PRINT_SIZE];
    stringify_perms(i.mode, perms);
    int length;

    fprintf(stderr, "\nFile inode:\n");

    /* Non-zone fields */
    length = (int) strlen("mode");
    fprintf(stderr, "  uint16_t %-*s %#*x (%s)\n", length, "mode",
                INODE_PRINT_ALIGN - length - 1, i.mode, perms);

    length = (int) strlen("links");
    fprintf(stderr, "  uint16_t %-*s %*u\n", length, "links",
                INODE_PRINT_ALIGN - length - 1, i.links);

    length = (int) strlen("uid");
    fprintf(stderr, "  uint16_t %-*s %*u\n", length, "uid",
                INODE_PRINT_ALIGN - length - 1, i.uid);

    length = (int) strlen("gid");
    fprintf(stderr, "  uint16_t %-*s %*u\n", length, "gid",
                INODE_PRINT_ALIGN - length - 1, i.gid);

    length = (int) strlen("size");
    fprintf(stderr, "  uint32_t %-*s %*u\n", length, "size",
                INODE_PRINT_ALIGN - length - 1, i.size);
    
    /* Prints out time fields */
    length = (int) strlen("atime");
    time_t t = (time_t) i.atime;
    fprintf(stderr, "  uint32_t %-*s %*u --- %s", length, "atime",
                INODE_PRINT_ALIGN - length - 1, i.atime, 
                ctime(&t));

    length = (int) strlen("mtime");
    t = (time_t) i.mtime;
    fprintf(stderr, "  uint32_t %-*s %*u --- %s", length, "mtime",
                INODE_PRINT_ALIGN - length - 1, i.mtime,
                ctime(&t));

    length = (int) strlen("ctime");
    t = (time_t) i.ctime;
    fprintf(stderr, "  uint32_t %-*s %*u --- %s", length, "ctime",
                INODE_PRINT_ALIGN - length - 1, i.ctime,
                ctime(&t));

    /* Print out zone meta data */
    fprintf(stderr, "\n  Direct zones:\n");
    
    print_zones(i);
    
    length = (int) strlen("indirect");
    fprintf(stderr, "%-*s%-*s %*u\n", ZONE_PRINT_PADDING, "  uint32_t",
                length, "indirect", 
                ZONE_PRINT_ALIGN - length - 1, i.indirect);
    length = (int) strlen("double");
    fprintf(stderr, "%-*s%-*s %*u\n", ZONE_PRINT_PADDING, "  uint32_t",
                length, "double",
                ZONE_PRINT_ALIGN - length - 1, i.two_indirect);
}

/* Prints out each of the fields of the given super block */
void print_sb(struct superblock sb) {
    int length;

    fprintf(stderr, "\nSuperblock Content\n");
    fprintf(stderr, "Stored Fields:\n");
    
    length = (int) strlen("ninodes");
    fprintf(stderr, "  %-*s %*u\n", length, "ninodes", 
                SB_PRINT_ALIGN - length - 1, sb.ninodes);
    length = (int) strlen("i_blocks");
    fprintf(stderr, "  %-*s %*d\n", length, "i_blocks",
                SB_PRINT_ALIGN - length - 1, sb.i_blocks);
    length = (int) strlen("z_blocks");
    fprintf(stderr, "  %-*s %*d\n", length, "z_blocks",
                SB_PRINT_ALIGN - length - 1, sb.z_blocks);
    length = (int) strlen("firstdata");
    fprintf(stderr, "  %-*s %*d\n", length, "firstdata",
                SB_PRINT_ALIGN - length - 1, sb.firstdata);
    length = (int) strlen("log_zone_size");
    fprintf(stderr, "  %-*s %*d (zone size: %d)\n", length,
                "log_zone_size", SB_PRINT_ALIGN - length - 1, 
                sb.log_zone_size, sb.blocksize << sb.log_zone_size);
    length = (int) strlen("max_file");
    fprintf(stderr, "  %-*s %*u\n", length, "max_file",
                SB_PRINT_ALIGN - length - 1, sb.max_file);
    length = (int) strlen("magic");
    fprintf(stderr, "  %-*s %#*x\n", length, "magic",
                SB_PRINT_ALIGN - length - 1, sb.magic);
    length = (int) strlen("zones");
    fprintf(stderr, "  %-*s %*u\n", length, "zones",
                SB_PRINT_ALIGN - length - 1, sb.zones);
    length = (int) strlen("blocksize");
    fprintf(stderr, "  %-*s %*u\n", length, "blocksize",
                SB_PRINT_ALIGN - length - 1, sb.blocksize);
    length = (int) strlen("subversion");
    fprintf(stderr, "  %-*s %*u\n", length, "subversion",
                SB_PRINT_ALIGN - length - 1, sb.subversion);
}

/* Reads the partition table of the image at the given base
 * Reads data into the given table of partition entries */
void read_partition_table(FILE *fp, long base, 
                            struct part_entry table[MAX_PARTS]) {
    uint16_t signature;
    size_t ret;
    
    /* First checks whether there is a valid partition table to read */
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

    /* Next reads the partition table into our given table */
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

/* Reads the superblock of the minix file system 
 * Returns the read super block */    
struct superblock read_superblock(FILE *fp, long base) {
    int ret;
    size_t nread;
    struct superblock sb;
    
    /* Reads the superblock from the image at the given base */
    ret = fseek(fp, base + SUP_BLOCK_OFFSET, SEEK_SET);
    if (ret) {
        perror("Failed superblock seek");
        exit(1);
    }
    
    nread = fread(&sb, sizeof(struct superblock), 1, fp);
    if (nread != 1) {
        perror("Failed superblock read");
        exit(1);
    }
    
    /* Checks whether the superblock is from a MINIX filesystem */
    if (sb.magic != MINIX_MAGIC) {
        perror("Not a MINIX filesystem");
        exit(1);
    }
    return sb;
}

/* Reads an inode from the inode table based on the given index 
 * Returns the read inode */
struct inode read_inode(FILE *fp, long base, uint32_t index, 
                            struct superblock sb) {
    long table_offset;
    long inode_offset;
    int ret;
    size_t nread;
    struct inode i;

    if (sb.ninodes < index) {
        perror("inode out of index");
        exit(1);
    }
    /* Calculates the location of the inode table */
    table_offset = (2 + sb.i_blocks + sb.z_blocks) * sb.blocksize;
    
    /* Calculates the location at the given index in the table */
    inode_offset = table_offset + (sizeof(struct inode) * (index - 1));

    ret = fseek(fp, base + inode_offset, SEEK_SET);
    if (ret) {
        perror("Failed inode seek");
        exit(1);
    }

    nread = fread(&i, sizeof(struct inode), 1, fp);
    if (nread != 1) {
        perror("Failed inode read");
        exit(1);
    }

    return i;
}

/* Reads the directory entries at the given list of zones
 * Reads data into the given table of directory entries
 * Returns the number of directory entries read across zones */
size_t read_dir_zones(FILE *fp, long base, uint32_t *zones, size_t nzones, 
                        size_t zone_size, struct dir_entry* table, 
                        uint32_t *remaining) {
    int ret;
    size_t nread;
    size_t entries;
    size_t cnt = 0;
    size_t cur_zone = 0;

    /* Loops until we run out of zones or directory entries to read */
    while (*remaining >= DIR_ENTRY_SIZE && cur_zone < nzones) {
        if (zones[cur_zone] == 0) {
            /* Treats the zone 0 as all zeroes 
             * Skips zone_size amount of bytes */
            if (*remaining < zone_size) {
                *remaining = 0;
                break;
            }
            cur_zone++;
            *remaining -= zone_size;
            continue;
        }

        ret = fseek(fp, base + zones[cur_zone] * zone_size, SEEK_SET);
        if (ret) {
            perror("Failed zone seek");
            exit(1);
        }
        
        if (*remaining < zone_size) {
            /* We only need to read part of the zone for our entries */
            entries = *remaining / DIR_ENTRY_SIZE;
            nread = fread(&table[cnt], DIR_ENTRY_SIZE, entries, fp);
            if (nread != entries) {
                perror("Failed partial zone read");
                exit(1);
            }

            cnt += entries;
            cur_zone++;
            *remaining = 0;
            break;
        }
        else {
            /* Read the full zone and subtract it from our remaining */
            entries = zone_size / DIR_ENTRY_SIZE;
            nread = fread(&table[cnt], DIR_ENTRY_SIZE, entries, fp);
            if (nread != entries) {
                perror("Failed full zone read");
                exit(1);
            }
            cnt += entries;
            cur_zone++;
            *remaining -= zone_size;
        }
    }
    return cnt;
}

/* Reads the directory entries in the given list of indirects
 * Reads data into the given table of directory entries
 * Returns the number of directory entries read across indirects */
size_t read_dir_indirects(FILE *fp, long base, uint32_t *indirects,
                            size_t zone_size, struct dir_entry *table,
                            uint32_t *remaining, size_t nindirect,
                            uint16_t blocksize) {
    int ret;
    size_t nread;
    size_t cnt = 0;
    size_t cur = 0;
    size_t nzones = blocksize / sizeof(uint32_t);
    uint32_t zones[nzones];
    
    while (*remaining >= DIR_ENTRY_SIZE && cur < nindirect) {
        if (indirects[cur] == 0) {
            /* Treats an indirect of 0 as full of zeros 
             * Skips the appropriate amount of bytes */
            if (*remaining < (nzones * zone_size)) {
                *remaining = 0;
                break;
            }
            cur++;
            *remaining -= nzones * zone_size;
            continue;
        }
        
        /* Reads the indirect as a list of zones */
        ret = fseek(fp, base + indirects[cur] * zone_size, SEEK_SET);
        if (ret) {
            perror("Failed indirect seek");
            exit(1);
        }

        nread = fread(zones, sizeof(uint32_t), nzones, fp);
        if (nread != nzones) {
            perror("Failed indirect read");
            exit(1);
        }
        
        /* Reads all zones of the indirect into the table */
        cnt += read_dir_zones(fp, base, zones, nzones, zone_size, &table[cnt],
                                remaining);
        cur++;
    }
    return cnt;
}

/* Reads the given inode as a directory
 * Reads data into the table of directory entries
 * Returns the amount of entries read across all zones, indirect, and double */
size_t read_dir(FILE *fp, long base, struct inode i, size_t zone_size, 
                struct dir_entry *table, uint16_t blocksize) { 
    int ret;
    size_t nread;
    size_t nindirect = blocksize / sizeof(uint32_t);
    uint32_t indirects[nindirect];
    size_t cnt = 0;
    uint32_t remaining = i.size;

    /* Reads all the direct zones as directory entries */
    cnt += read_dir_zones(fp, base, i.zone, DIRECT_ZONES, zone_size, 
                            table, &remaining);

    if (remaining >= DIR_ENTRY_SIZE) {
        /* If are expecting more directory entries, then we read
         * the indirect for more directory entries */
        cnt += read_dir_indirects(fp, base, &i.indirect, zone_size, 
                                    &table[cnt], &remaining, 1, blocksize);
    }
    if (remaining >= DIR_ENTRY_SIZE) {
        /* If we are still expecting more directory entries, then we read
         * the double for the rest of the directory entries */
        if (i.two_indirect == 0) {
            perror("Exceeded readable filesize");
            exit(1);
        }

        /* Reads the double as a list of indirects */
        ret = fseek(fp, base + i.two_indirect * zone_size, SEEK_SET);
        if (ret) {
            perror("Failed two-indirect seek");
            exit(1);
        }

        nread = fread(indirects, sizeof(uint32_t), nindirect, fp);
        if (nread != nindirect) {
            perror("Failed two-indirect read");
            exit(1);
        }
        
        /* Reads the list of indirects as directory entries */
        cnt += read_dir_indirects(fp, base, indirects, zone_size, &table[cnt],
                                    &remaining, nindirect, blocksize);
        
        if (remaining >= DIR_ENTRY_SIZE) {
            perror("Exceeded readable filesize");
            exit(1);
        }
    }
    return cnt;
}

/* Navigates the file system based on the given path
 * Repeatedly reads the current inode as a directory to find a matching
 * name entry to the given path, which is read as the next inode
 * Returns the directory entry of the final inode of the path */
struct dir_entry navigate_fs(FILE *fp, long base, struct superblock sb,
                    char *path) {
    char *saveptr;
    size_t i;
    struct inode cur_inode;
    size_t n_entries;
    uint32_t cur_idx = 1; /* Navigation starts at root */
    struct dir_entry cur_entry;

    char *temppath = strdup(path);
    if (!temppath) {
        perror("Failed to duplicate fs path");
        exit(1);
    }    
    
    char *target = strtok_r(temppath, "/", &saveptr);
    
    if (target == NULL) {
        /* Case when the path is the root inode */
        cur_entry.inode = 1;
        strncpy((char *)cur_entry.name, "/", MAX_NAME);
        free(temppath);
        return cur_entry;
    }
    
    while(target != NULL) {
        int found = !FOUND;
        cur_inode = read_inode(fp, base, cur_idx, sb);
        
        if ((cur_inode.mode & TYPE_MASK) != DIR_MASK) {
            /* Only traverses files that are directories */
            free(temppath);
            perror("Not a Directory");
            exit(1);
        }

        struct dir_entry table[cur_inode.size / DIR_ENTRY_SIZE];

        /* Reads zones of the current inode as a directory */
        n_entries = read_dir(fp, base, cur_inode, 
                                sb.blocksize << sb.log_zone_size,
                                table, sb.blocksize);

        for (i=0; i < n_entries; i++) {
            /* Each entry in the current directory is matched against
             * the path */
            cur_entry = table[i];
            
            if (strncmp((char *) table[i].name, target, MAX_NAME) == 0
                    && table[i].inode != 0) {
                cur_idx = table[i].inode;
                found = FOUND;
                break;
            }
        }
        
        if (!found) {
            free(temppath);
            perror("Directory not found");
            exit(1);
        }

        target = strtok_r(NULL, "/", &saveptr);
    }
    
    
    free(temppath);
    return cur_entry;
}

/* An error checker wrapper for strtol */
long my_strtol(char *str) {
    char *end;
    errno = 0;

    long result = strtol(str, &end, 10);

    if (errno != 0) {
        perror("Failed strtol");
        exit(1);
    }

    if (*end != '\0') {
        perror("Invalid input, expected an integer");
        exit(1);
    }

    return result;
}

/* Calculates the base of the file system after reading and checking
 * the given partion and subpartition */
long get_base(FILE *fp, long part, long sub, int verbose) {
    struct part_entry table[MAX_PARTS];
    long base = 0;

    if (part != INVALID_PART) {
        read_partition_table(fp, base, table);
        
        if (table[part].type != MINIX_TYPE) {
            perror("Not a MINIX partition");
            exit(1);
        }

        base = table[part].lFirst * SECTOR_SIZE;
    
        if(sub != INVALID_PART) {
            read_partition_table(fp, base, table);
            
            if (table[sub].type != MINIX_TYPE) {
                perror("Not a MINIX subpartition");
                exit(1);
            }

            base = table[sub].lFirst * SECTOR_SIZE;
        }
    
    }
    return base;
}        
