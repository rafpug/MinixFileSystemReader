#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "partition-reader.h"

/* minget.c serves as the source file for the minget cmd
 * 
 * minget copies a regular file from the given source path to the given
 * destination path
 *
 * if nodestination path is given, then the file is copied to stdout */



/* Error checking wrapper for fwrite */
void wrap_fwrite(void *ptr, size_t size, size_t nmemb, FILE *fp) {
    size_t nwritten;
    nwritten = fwrite(ptr, size, nmemb, fp);

    if (nwritten != nmemb) {
        perror("Failed to write to destination");
        exit(1);
    }
}

/* Reads the list of given zones from the image and writes them into
 * the destination file
 * Returns the leftover number of bytes */
uint32_t rw_reg_zones(FILE *image_fp, long base, uint32_t *zones, 
                            size_t nzones, size_t zone_size, 
                            uint32_t remaining, FILE *dst_fp) {
    int ret;
    size_t nread;
    size_t nwritten;
    size_t cur_zone = 0;
    unsigned char buf[zone_size];
    
    /* Loops until no more zones or no more expected data */
    while (remaining && cur_zone < nzones) {
        if (zones[cur_zon] == 0) {
            /* Any zone# 0 is treated as full of zero
             * so we write the chunk of '\0' to the dst file */
    
            memset(buf, 0, zone_size);
            
            if (remaining < zone_size) {
                wrap_fwrite(buf, sizeof(char), remaining, dst_fp);
                remaining = 0;
                break;
            }
            else {
                wrap_fwrite(buf, sizeof(char), zone_size, dst_fp);
                cur_zone++;
                remaining -= zone_size;
                continue;
            }
        }

        ret = fseek(image_fp, base + zones[cur_zone] * zone_size, SEEK_SET);

        if (ret) {
            perror("Failed minget zone seek");
            exit(1);
        }

        if (remaining < zone_size) {
            /* Only part of the zone needs to be read for the data */
            
            nread = fread(buf, sizeof(char), remaining, image_fp);
            if (nread != remaining) {
                perror("Failed minget partial zone read");
                exit(1);
            }
            
            wrap_fwrite(buf, sizeof(char), nread, dst_fp);

            cur_zone++;
            remaining = 0;
            break;
        }
        else {
            /* Reads the full zone worth of data */
            
            nread = fread(buf, sizeof(char), zone_size, image_fp);
            if (nread != zone_size) {
                perror("Failed minget full zone read");
                exit(1);
            }

            wrap_fwrite(buf, sizeof(char), nread, dst_fp);

            cur_zone++;
            remaining -= zone_size;
        }
    }
    return remaining;
}

/* Reads the data from the given list of indirects from the given image and
 * writes them into the given destination file
 * Returns the amount of remaining data */
uint32_t rw_reg_indirects(FILE *image_fp, long base, uint32_t *indirects,
                            size_t zone_size, uint32_t remaining, 
                            size_t nindirect, uint16_t blocksize, 
                            FILE *dst_fp) {
    int ret,
    size_t nread;
    size_t cur_indirect = 0;
    size_t nzones = blocksize / sizeof(uint32_t);
    uint32_t zones[nzones];
        

    while (remaining && cur < nindirect) {
        if (indirects[cur] == 0) {
            /* Writes an entire indirect block worth of zeros
             * up to the remaining amount of data */
            memset(buf, 0, nzones * zone_size);

            if (remaining < (nzones * zone_size)) {
                wrap_fwrite(buf, sizeof(char), remaining, dst_fp);
                remaining = 0;
                break;
            }
            else {
                wrap_fwrite(buf, sizeof(char), zone_size, dst_fp);
                cur_indirect++;
                remaining -= nzones * zone_size;
                continue;
            }
        }
        
        /* Indirect is read as a list of zones */
        ret = fseek(image_fp, base + indirects[cur_indirect] * zone_size,
                        SEEK_SET);
        if (ret) {
            perror("Failed minget indirect seek");
            exit(1);
        }

        nread = fread(zones, sizeof(uint32_t), nzones, image_fp);
        if (nread != nzones) {
            perror("Failed minget indirect read");
            exit(1);
        }
            
        remaining = rw_reg_zones(image_fp, base, zones, nzones, zone_size
                                    remaining, dst_fp);
    
        cur_indirect++;
    }
    return remaining;                       
}

/* Reads the given inode as a regular file
 * Writes the read data into the destination file */
void rw_reg(FILE *image_fp, long base, struct inode i, size_t zone_size,
                uint16_t blocksize, FILE *dst_fp) {
    int ret;
    size_tnread;
    size_t nindirect = blocksize / sizeof(uint32_t);
    uint32_t indirects[nindirect];
    uint32_t remaining = i.size;
    
    remaining = rw_reg_zones(image_fp, base, i.zone, DIRECT_ZONES, zone_size,
                                remaining, dst_fp);

    if (remaining) {
        /* Reads and writes the indirect for any leftover data */
        remaining = rw_reg_indirects(image_fp, base, &i.indirect, zone_size,
                                        remaining, nindirect, blocksize,
                                        dst_fp);
    }
    if (remaining) {
        /* Reads and writes the double for any leftover data */
        if (i.two_indirect == 0) {
            perror("minget exceeded implemented filesize");
            exit(1);
        }
            
        ret = fseek(image_fp, base + i.two_indirect * zone_size, SEEK_SET);
        if (ret) {
            perror("Failed minget two-indirect seek");
            exit(1);
        }
    
        nread = fread(indirects, sizeof(uint32_t), nindirect, image_fp);
        if (nread != nindirect) {
            perror("Failed minget two-indirect read");
            exit(1);
        }
    
        remaining = rw_reg_indirect(image_fp, base, indirects, zone_size
                                        remaining, nindirect, blocksize,
                                        dst_fp);

        if (remaining) {
            perror("minget exceeded double") {
            exit(1);
        }
    }
}


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
                return 1;
        }
    }

    if (optind >= argc) {
        perror("Some help");
        return 1;
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
        if (dst_fp != stdout) {
            free(dst_fp);
        }
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
        rw_reg(image_fp, base, src, sb.blocksize << sb.log_zone_size,
                sb.blocksize, dst_fp);
    }
    else {
        fclose(image_fp);
        if (dst_fp != stdout) {
            fclose(dst_fp);
        }
        perror("Not a regular file");
        exit(1);
    }
    return 0;
}
