/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern struct disk * thedisk;
int * bitmap;

int fs_format()
{
    // FIXME: how do we check if the disk is already mounted?

    // Write superblock
    union fs_block block;
    block.super.magic = FS_MAGIC;
    block.super.nblocks = disk_size();

    // Set aside 10% of blocks for inodes
    block.super.ninodeblocks = block.super.nblocks / 10;
    if (block.super.nblocks % 10 > 0) {
        block.super.ninodeblocks++;
    }

    // Write number of inodes
    block.super.ninodes = block.super.ninodeblocks * INODES_PER_BLOCK;

    // Clear blocks, initialize to 0
    union fs_block next_block;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        next_block.data[i] = 0;
    }

    for (int i = 1; i < block.super.nblocks; i++) {
        disk_write(thedisk, i, next_block.data);
    }

    return 1;
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
            int inumber = ((i-1) * INODES_PER_BLOCK) + j;

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
    bitmap = calloc(block.super.nblocks, sizeof(int));
    for (int i = 0; i < block.super.nblocks; i++) {
        bitmap[i] = 0; // 0 ~ unused block
    }
    bitmap[0] = 1; // 1 ~ used block

    printf("BEFORE\n");
    for (int i = 0; i < block.super.nblocks; i++) {
        printf("%d\n", bitmap[i]);
    }

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

    printf("AFTER\n");
    for (int i = 0; i < block.super.nblocks; i++) {
        printf("%d\n", bitmap[i]);
    }

    return 1;
}

int fs_create()
{
    // Read inode blocks
    union fs_block block;
    disk_read(thedisk, 0, block.data);

    // Loop over inode blocks
    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        
        union fs_block next_block;
        disk_read(thedisk, i, next_block.data);

        // Loop over inodes in block, search for free inode
        for (int j = 1; j < INODES_PER_BLOCK; j++) {

            // Initialize and return inumber
            if (next_block.inode[j].isvalid == 0) {
                
                next_block.inode[j].isvalid = 1;
                next_block.inode[j].size = 0;

                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    next_block.inode[j].direct[k] = 0;
                }

                next_block.inode[j].indirect = 0;

                disk_write(thedisk, i, next_block.data);

                return ((i-1) * INODES_PER_BLOCK) + j;
            }
        }
    }

    return 0;
}

int fs_delete( int inumber )
{
    // Read inode block
    size_t iblock = inumber/INODES_PER_BLOCK + 1;
    size_t ioffset = inumber % INODES_PER_BLOCK;
    
    union fs_block block;
    disk_read(thedisk, iblock, block.data);
    
    if (block.inode[ioffset].isvalid == 0) {
        return 0;
    }

    // Release direct blocks
    for (int i = 0; i < POINTERS_PER_INODE; i++) {

        // Update bitmap
        if (block.inode[ioffset].direct[i] > 0 && block.inode[ioffset].direct[i] < disk_size()) {
            bitmap[block.inode[ioffset].direct[i]] = 0;
        }

        block.inode[ioffset].direct[i] = 0;
    }

    // Relase indirect pointers
    if (block.inode[ioffset].indirect > 0 && block.inode[ioffset].indirect < disk_size()) {
        
        // Read indirect block
        union fs_block next_block;
        disk_read(thedisk, block.inode[ioffset].indirect, next_block.data);

        // Loop over pointers in indirect block, update bitmap
        for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
            
            if (next_block.pointers[i] > 0 && next_block.pointers[i] < disk_size()) {
                bitmap[next_block.pointers[i]] = 0;
            }

            next_block.pointers[i] = 0;
        }

        // Update bitmap
        bitmap[block.inode[ioffset].indirect] = 0;
        block.inode[ioffset].indirect = 0;
    }

    // Mark inode as free
    block.inode[ioffset].isvalid = 0;
    block.inode[ioffset].size = 0;

    disk_write(thedisk, iblock, block.data);

	return 1;
}

int fs_getsize( int inumber )
{
    // Read inode block
    size_t iblock = inumber/INODES_PER_BLOCK + 1;
    size_t ioffset = inumber % INODES_PER_BLOCK;

    union fs_block block;
    disk_read(thedisk, iblock, block.data);

    if (block.inode[ioffset].isvalid == 0) {
        return -1;
    }

    // Return size of inode
	return block.inode[ioffset].size;
}

int fs_read( int inumber, char *data, int length, int offset )
{
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}