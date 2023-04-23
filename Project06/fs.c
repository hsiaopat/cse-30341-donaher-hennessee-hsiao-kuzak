
#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024

struct fs_superblock {
	int magic;
	int nblocks;
	int ninodeblocks;
	int ninodes;
};

struct fs_inode {
	int isvalid;
	int size;
	int direct[POINTERS_PER_INODE];
	int indirect;
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	int pointers[POINTERS_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};

int fs_format()
{
	return 0;
}

void fs_debug()
{
	union fs_block block;

    // Read superblock from disk
    disk_read(0, block.data);

    // Validate magic number
    if (block.super.magic != FS_MAGIC) {
        printf("ERROR: Invalid magic number in superblock.\n");
        return;
    }

    printf("superblock:\n");
    printf("    %d blocks\n", block.super.nblocks);
    printf("    %d inode blocks\n", block.super.ninodeblocks);
    printf("    %d inodes\n", block.super.ninodes);

    // Iterate through all inode blocks and print information about each inode
    int inode_blocks = block.super.ninodeblocks;

    for (int i = 1; i <= inode_blocks; i++) {

		union fs_block next_block;
        disk_read(i, next_block.data);

        for (int j = 0; j < INODES_PER_BLOCK; j++) {

            struct fs_inode * inode = &next_block.inode[j];
            int inumber = ((i - 1) * INODES_PER_BLOCK) + j;

            if (inode->isvalid) {
			
                printf("inode %d:\n", inumber);
                printf("    size: %d bytes\n", inode->size);
                printf("    direct blocks:");

                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    if (inode->direct[k] != 0) {
                        printf(" %d", inode->direct[k]);
                    }
                }
                printf("\n");

                if (inode->indirect != 0) {

                    printf("    indirect block: %d\n", inode->indirect);
                    printf("    indirect data blocks:");

                    for (int k = 0; k < POINTERS_PER_BLOCK; k++) {
                        if (next_block.pointers[k] != 0) {
                            printf(" %d", next_block.pointers[k]);
                        }
                    }
                    printf("\n");
                }
            }
        }
    }
}

int fs_mount()
{
	return 0;
}

int fs_create()
{
	return 0;
}

int fs_delete( int inumber )
{
	return 0;
}

int fs_getsize( int inumber )
{
	return -1;
}

int fs_read( int inumber, char *data, int length, int offset )
{
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}
