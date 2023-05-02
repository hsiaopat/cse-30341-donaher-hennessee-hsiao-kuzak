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
    // Read superblock from disk
    union fs_block block;
    disk_read(thedisk, 0, block.data);

    // Validate magic number
    if (block.super.magic != FS_MAGIC) {
        printf("ERROR: Invalid magic number in superblock.\n");
        return 0;
    }

    // Check the disk block size 
    if (disk_size() != block.super.nblocks) {
        printf("ERROR: Disk block size does not match superblock.\n");
        return 0;
    }

    // Build free block bitmap // FIXME: finish with error handling for number of inodes
    // int bitmap_size = block.super.nblocks / 10;
    // if (block.super.nblocks % 10 > 0) {
    //     bitmap_size++;
    // }

    // Initialize bitmap
    int bitmap[block.super.nblocks];
    for (int i = 0; i < block.super.nblocks; i++) {
        bitmap[i] = 0; // 0 ~ unused block
    }
    bitmap[0] = 1; // 1 ~ used block

    // Loop over inode blocks
    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        
        bitmap[i] = 1;
        
        // Read current inode block from disk
		union fs_block next_block;
        disk_read(thedisk, i, next_block.data);

        // Loop over all inodes in a block
        for (int j = 0; j < INODES_PER_BLOCK; j++) {

            struct fs_inode * inode = &next_block.inode[j];

            // Check if inode is valid
            if (inode->isvalid) {

                // Check direct blocks
                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    if (inode->direct[k] > 0 && inode->direct[k] < disk_size()) { 
                        bitmap[inode->direct[k]] = 1;
                    }
                }

                // Check if indirect pointer is valid
                if (inode->indirect > 0 && inode->indirect < disk_size()) {

                    bitmap[inode->indirect] = 1;

                    // Read indirect block from disk
                    union fs_block indirect_block;
                    disk_read(thedisk, inode->indirect, indirect_block.data);

                    // Loop over pointers in indirect block
                    for (int k = 0; k < POINTERS_PER_BLOCK; k++) {
                        if (indirect_block.pointers[k] > 0 && indirect_block.pointers[k] < disk_size()) {
                            bitmap[indirect_block.pointers[k]] = 1;
                        }
                    }
                }
            }
        }
    }

    return 1;
}

int fs_create()
{
	return 0;
}

int fs_delete( int inumber )
{
    // // Read superblock from disk
    // union fs_block super_block;
    // disk_read(thedisk, 0, super_block.data);

    // // Check if disk is mounted
    // if (disk->is_mounted) { //DOES NOT WORK IS MOUNTED IS NOT A THING CAN WE ADD IT?
    //     printf("ERROR: Disk is not mounted.\n");
    //     return 0;
    // }

    // // Check if inumber is valid
    // if (inumber <= 0 || inumber >= super_block.super.ninodes) {
    //     printf("ERROR: Invalid inode number.\n");
    //     return 0;
    // }

    // // Read inode from disk
    // int inode_block_index = (inumber - 1) / INODES_PER_BLOCK + 1;
    // int inode_index_within_block = (inumber - 1) % INODES_PER_BLOCK;
    // union fs_block inode_block;
    // disk_read(thedisk, inode_block_index, inode_block.data);
    // struct fs_inode* inode = &inode_block.inode[inode_index_within_block];

    // // Check if inode is valid
    // if (!inode->isvalid) {
    //     printf("ERROR: Inode does not exist.\n");
    //     return 0;
    // }

    // // Free direct blocks
    // for (int i = 0; i < POINTERS_PER_INODE; i++) {
    //     if (inode->direct[i] != 0) {
    //         disk_set_block(inode->direct[i], 0);
    //         inode->direct[i] = 0;
    //     }
    // }

    // // Free indirect blocks
    // if (inode->indirect != 0) {
    //     union fs_block indirect_block;
    //     disk_read(thedisk, inode->indirect, indirect_block.data);
    //     for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
    //         if (indirect_block.pointers[i] != 0) {
    //             disk_set_block(indirect_block.pointers[i], 0);
    //             indirect_block.pointers[i] = 0;
    //         }
    //     }
    //     disk_set_block(inode->indirect, 0);
    //     inode->indirect = 0;
    // }

    // // Update inode on disk
    // inode->isvalid = 0;
    // disk_write(thedisk, inode_block_index, inode_block.data);

    // // Return blocks to free block map DOES NOT WORK free????
    // union fs_block bitmap_block;
    // for (int i = 1; i < super_block.super.nblocks; i++) {
    //     disk_read(thedisk, i, bitmap_block.data);
    //     if (bitmap_block.free == 0) {
    //         bitmap_block.free = 1;
    //         disk_write(thedisk, i, bitmap_block.data);
    //     }
    // }

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