#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "partition-reader.h"

int main(int argc, char **argv) {
    int verbose = !VERBOSE;
    long part = INVALID_PART;
    long sub = INVALID_SUB;
    char *image_path;
    char *src_path;
    char *dst_path = NULL;
    int opt;
    struct part_entry table[MAX_PARTS];
    struct superblock sb;
    long base = 0;
    FILE *image_fp;
    FILE *dst_fp = NULL;
    
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
                perror("help");
                exit(1);
        }
    }

    if (optind >= argc) {
        perror("Some help");
        exit(1);
    }

    if (part >= MAX_PARTS || sub >= MAX_PARTS) {
        fprintf(stderr, "Invalid parition index\m");
        return 1;
    }
    
    if (optind < argc) {
        image_path = arv[optind++];
    }
    else {
        fprintf(stderr, "Missing image path");
        return 1;
    }

    if (optind < argc) {
        src_path = argv[optind++];
    }
    else {
        fprintf(stderr, "Missing src path");
        return 1;
    }

    if (optind < argc) {
        dst_path = argv[optind];
        dst_fp = fopen(dst_path, "wb");
    } else {
        dst_fp = stdout;
    }

    if (!dst_fp) {
        perror("Failed to open destination");
        exit(1);
    }
    
    image_fp = fopen(image_path, "rb");
    
    if (!image_fp) {
        perror("Failed to open image");
        exit(1);
    }
    
    base = get_base(image_fp, part, sub, verbose);
    
    sb = read_superblock(image_fp, base);
    
    struct dir_entry src_entry = navigate_fs(image_fp, base, sb, src_path);
    struct inode src = read_inode(image_fp, base, src_entry.inode, sb);
    
    if (verbose) {
        print_sb(sb);
        print_inode(dest);
    }
    
    if ((dest.mode & TYPE_MASK) == REG_MASK) {
        
    }
    else {
        perror("Not a regular file");
        exit(1);
    }
    return 0;
}
