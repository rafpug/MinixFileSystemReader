#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "partition-reader.h"

/* minls.c serves as the source file for the minls command
 *
 * minls lists a file or directory on the given filesystem image
 * The optional path determines what is listed
 * If no path is given, then root is  */

/* Prints out the ls of the corresponding directory entry */
void print_ls(struct inode i, char *name) {
    char perms[PERM_PRINT_SIZE];
    stringify_perms(i.mode, perms);
 
    printf("%s %9u %s\n", perms, i.size, name);
}

/* Cleans up the path into a standard format 
 * Caller guarantees the new string is 2 chars longer than old*/
void clean_path(char *old, char *new) {
    char *saveptr;
    int cur = 0;
    char *target = strtok_r(old, "/", &saveptr);

    while(target != NULL) {
        /* Ensures every path starts with '/' and no duplicate '/' */
        new[cur++] = '/';
        strncpy(new + cur, target, strlen(target));
        cur += strlen(target);
        target = strtok_r(NULL, "/", &saveptr);
    }

    if (cur == 0) {
        new[cur++] = '/';
    }

    new[cur] = '\0';
}       
        
int main(int argc, char **argv) {
    int verbose = !VERBOSE;
    long part = INVALID_PART;
    long sub = INVALID_SUB;
    char *image_path;
    char *fs_path;
    int opt;
    struct superblock sb;
    long base = 0;

    while ((opt = getopt(argc, argv, "vp:s:h")) != -1) {
        switch (opt) {
            case 'v':
                verbose = VERBOSE;
                break;
            case 'p':
                part = my_strtol(optarg);
                break;
            case 's':
                sub = my_strtol(optarg);
                break;
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
    
    if (optind < argc) { 
        image_path = argv[optind++];
    }
    else {
        perror("Not given image");
        exit(1);
    }

    if (optind < argc) {
        fs_path = argv[optind];
    }
    else {
        fs_path =  "/";
    }
    
    FILE *fp = fopen(image_path, "rb");
    
    if (!fp) {
        perror("Failed to open image");
        exit(1);
    }   
    
    /* First gets the base of the file system */
    base = get_base(fp, part, sub, verbose);

    /* Then reads the superblock with that base */
    sb = read_superblock(fp, base);

    /* Next we can navigate the file system with sb and the path */
    struct dir_entry dest_entry = navigate_fs(fp, base, sb, fs_path);

    /* Get the metadata for file at the end of the path */
    struct inode dest = read_inode(fp, base, dest_entry.inode, sb);
    
    /* Cleans up the path for printouts */
    char cleaned_path[strlen(fs_path) + 2];
    cleaned_path[strlen(fs_path)] = '\0';

    char *fs_path_copy = strdup(fs_path);

    if (!fs_path_copy) {
        perror("Failed to duplicate string");
        exit(1);
    }
    clean_path(fs_path_copy, cleaned_path);
    free(fs_path_copy);
   
    if (verbose) {
        print_sb(sb);
        print_inode(dest);
    }
 
    if ((dest.mode & TYPE_MASK) == DIR_MASK) {
        /* If the file at the end of the path is a directory, then we print
         * the ls of all the files under it */
        printf("%s:\n", cleaned_path);
        
        struct dir_entry entries[dest.size / DIR_ENTRY_SIZE];
        size_t n_entries = read_dir(fp, base, dest, 
                                        sb.blocksize << sb.log_zone_size, 
                                        entries, sb.blocksize);
        size_t i;
        for (i = 0; i < n_entries; i++) {
            if (entries[i].inode == 0) {
                continue;
            }
            struct inode cur_inode = read_inode(fp, base, entries[i].inode, 
                                                    sb);
            char name[MAX_NAME + 1];
            name[MAX_NAME] = '\0';
            strncpy(name, (char *)entries[i].name, MAX_NAME);
            
            print_ls(cur_inode, name);
        }
    }
    else if ((dest.mode & TYPE_MASK) == REG_MASK) {
        /* If its just a regular file then we just print the ls of it alone */
        print_ls(dest, cleaned_path + 1);
    }
    
    fclose(fp);
    return 0;
}

    
    

