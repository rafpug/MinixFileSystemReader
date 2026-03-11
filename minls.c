#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "partition-reader.h"

void print_inode(struct inode i, char *name) {
    char perms[PERM_PRINT_SIZE];
    perms[PERM_PRINT_SIZE - 1] = '\0';
    
    uint16_t type = i.mode & TYPE_MASK;
    if (type == REG_MASK) {
        perms[0] = 'r';
    }
    else if (type == DIR_MASK) {
        perms[0] = 'd';
    }
    else {
        perms[0] = '-';
    }

    perms[1] = (i.mode & OWNER_R_PERM) ? 'r' : '-';
    perms[2] = (i.mode & OWNER_W_PERM) ? 'w' : '-';
    perms[3] = (i.mode & OWNER_X_PERM) ? 'x' : '-';
    perms[4] = (i.mode & GROUP_R_PERM) ? 'r' : '-';
    perms[5] = (i.mode & GROUP_W_PERM) ? 'w' : '-';
    perms[6] = (i.mode & GROUP_X_PERM) ? 'x' : '-';
    perms[7] = (i.mode & OTHER_R_PERM) ? 'r' : '-';
    perms[8] = (i.mode & OTHER_W_PERM) ? 'w' : '-';
    perms[9] = (i.mode & OTHER_X_PERM) ? 'x' : '-';
    
    printf("%s %9u %s\n", perms, i.size, name);
}

void clean_path(char *old, char *new) {
    char *saveptr;
    int cur = 0;
    char *target = strtok_r(old, "/", &saveptr);

    while(target != NULL) {
        new[cur++] = '/';
        strncpy(new + cur, target, strlen(target));
        target = strtok_r(NULL, "/", &saveptr);
    }

    if (cur == 0) {
        new[cur] = '/';
    }
}       
        

int main(int argc, char **argv) {
    int verbose = !VERBOSE;
    long part = INVALID_PART;
    long sub = INVALID_SUB;
    char *image_path;
    char *fs_path;
    int opt;
    struct part_entry table[MAX_PARTS];
    struct superblock sb;
    long base = 0;

    while ((opt = getopt(argc, argv, "vp:s:h")) != -1) {
        switch (opt) {
            case 'v':
                verbose = VERBOSE;
            case 'p':
                part = my_strtol(optarg);
            case 's':
                sub = my_strtol(optarg);
            case 'h':
            default:
                perror("Some help");
                exit(1);
        }
    }

    if (optind >= argc) {
        perror("Some help");
        exit(1);
    }

    if (part >= MAX_PARTS || sub >= MAX_PARTS) {
        perror("Invalid partition index");
        exit(1);
    }
    
    image_path = argv[optind++];

    if (optind < argc) {
        fs_path = argv[optind];
    }
    else {
        fs_path =  "/";
    }
    
    FILE *fp = fopen(image_path, "rb");
    
    if(part != INVALID_PART) {
 
        read_partition_table(fp, base, table);
        base += table[part].lFirst * SECTOR_SIZE;
        
        if(sub != INVALID_PART) {
            read_partition_table(fp, base, table);
            base += table[sub].lFirst * SECTOR_SIZE;
        }
    }

    sb = read_superblock(fp, base);

    struct dir_entry dest_entry = navigate_fs(fp, base, sb, fs_path);
    struct inode dest = read_inode(fp, base, dest_entry.inode, sb);
    
    char cleaned_path[strlen(fs_path) + 1];
    cleaned_path[strlen(fs_path)] = '\0';
    clean_path(fs_path, cleaned_path);
    
    if ((dest.mode & TYPE_MASK) == DIR_MASK) {
        printf("%s:", cleaned_path);
        
        struct dir_entry entries[dest.size / DIR_ENTRY_SIZE];
        size_t n_entries = read_dir(fp, base, dest, 
                                        sb.blocksize << sb.log_zone_size, 
                                        entries);
        size_t i;
        for (i = 0; i < n_entries; i++) {
            struct inode cur_inode = read_inode(fp, base, entries[i].inode, 
                                                    sb);
            char name[MAX_NAME + 1];
            name[MAX_NAME] = '\0';
            strncpy(name, (char *)entries[i].name, MAX_NAME);
            
            print_inode(cur_inode, name);
        }
    }
    else if ((dest.mode & TYPE_MASK) == REG_MASK) {
        print_inode(dest, cleaned_path);
    }
    return 0;
}

    
    

