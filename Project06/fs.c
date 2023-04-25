/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdint.h>

extern struct disk *thedisk;

int fs_format()
{
	return 0;
}

void fs_debug()
{
    // Read superblock from disk
    union fs_block block;
    disk_read(thedisk, 0, block.data);

    // Validate magic number
    if (block.super.magic != FS_MAGIC) {
        printf("ERROR: Invalid magic number in superblock.\n");
        return;
    }

    // Checking the disk block size 
    if (disk_size() != block.super.nblocks) {
        printf("ERROR: Disk block size does not match superblock.\n");
        return;
    }

    // Display superblock info
    printf("superblock:\n");
    printf("    %d blocks\n", block.super.nblocks);
    printf("    %d inode blocks\n", block.super.ninodeblocks);
    printf("    %d inodes\n", block.super.ninodes);

    // Loop over inode blocks
    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        
        // Read current inode block from disk
		union fs_block next_block;
        disk_read(thedisk, i, next_block.data);

        // Loop over all inodes in a block
        for (int j = 0; j < INODES_PER_BLOCK; j++) {

            struct fs_inode * inode = &next_block.inode[j];
            int inumber = ((i - 1) * INODES_PER_BLOCK) + j;

            // Check if inode is valid
            if (inode->isvalid) {
                
                // Display inode info
                printf("inode %d:\n", inumber);
                printf("    size: %d bytes\n", inode->size);

                // Loop over direct pointers in inode
                printf("    direct blocks:");
                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    if (inode->direct[k] > 0 && inode->direct[k] < disk_size()) {    
                        printf(" %d", inode->direct[k]);
                    }
                }   
                printf("\n");

                // Check if indirect pointer is valid
                if (inode->indirect > 0 && inode->indirect < disk_size()) {

                    // Read indirect block from disk
                    union fs_block indirect_block;
                    disk_read(thedisk, inode->indirect, indirect_block.data);

                    // Loop over pointers in indirect block
                    printf("    indirect block: %d\n", inode->indirect);
                    printf("    indirect data blocks:");
                    for (int k = 0; k < POINTERS_PER_BLOCK; k++) {
                        if (indirect_block.pointers[k] > 0 && indirect_block.pointers[k] < disk_size()) {
                            printf(" %d", indirect_block.pointers[k]);
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