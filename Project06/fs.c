/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

    // Release indirect pointers
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
    size_t bytes_read = 0;

    // Load inode
    size_t iblock = inumber/INODES_PER_BLOCK + 1;
    size_t ioffset = inumber % INODES_PER_BLOCK;

    union fs_block block;
    disk_read(thedisk, iblock, block.data);

    // Check if inode is valid
    if (block.inode[ioffset].isvalid == 0) {
        return 0;
    }

    // Calculate logical block
    size_t logical_block = offset / BLOCK_SIZE;
    size_t next_block_offset = offset % BLOCK_SIZE;

    // Truncate length to read if larger than inode size
    if (length + offset > block.inode[ioffset].size) {
        length = block.inode[ioffset].size - offset;
    }

    // Calculate length to read from each inode
    size_t to_read;
    if (length + next_block_offset < BLOCK_SIZE) {
        to_read = length;
    }
    else {
        to_read = BLOCK_SIZE - next_block_offset;
    }

    // Read blocks
    while (bytes_read < length && logical_block < POINTERS_PER_BLOCK + POINTERS_PER_INODE) {
        
        // If direct...
        if (logical_block < POINTERS_PER_INODE) {

            // Read direct block
            union fs_block direct_block;
            disk_read(thedisk, block.inode[ioffset].direct[logical_block], direct_block.data);

            // Copy to data buffer
            memcpy(data + bytes_read, direct_block.data + next_block_offset, to_read);
            bytes_read += to_read;
        }

        // If indirect...
        else {

            // Read indirect block
            union fs_block indirect_block;
            disk_read(thedisk, block.inode[ioffset].indirect, indirect_block.data);

            // Read pointers
            union fs_block next_block;
            disk_read(thedisk, indirect_block.pointers[logical_block - POINTERS_PER_INODE], next_block.data);

            // Copy to data buffer
            memcpy(data + bytes_read, next_block.data + next_block_offset, to_read);
            bytes_read += to_read;
        }
    }
    
	return bytes_read;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
    //initialize for bitmap search
    int found = 0;


    size_t bytes_written = 0;

    // Load inode
    size_t iblock = inumber/INODES_PER_BLOCK + 1;
    size_t ioffset = inumber % INODES_PER_BLOCK;

    union fs_block block;
    disk_read(thedisk, iblock, block.data);

    // Check if inode is valid
    if (block.inode[ioffset].isvalid == 0) {
        return 0;
    }

    // Calculate logical block
    size_t logical_block = offset / BLOCK_SIZE;
    size_t next_block_offset = offset % BLOCK_SIZE;

    // Truncate length to read if larger than inode size
    if (length + offset > block.inode[ioffset].size) {
        length = block.inode[ioffset].size - offset;
    }

    // Calculate length to write from each inode
    size_t to_write;
    if (length + next_block_offset < BLOCK_SIZE) {
        to_write = length;
    }
    else {
        to_write = BLOCK_SIZE - next_block_offset;
    }

    // Write blocks
    while (bytes_written < length && logical_block < POINTERS_PER_BLOCK + POINTERS_PER_INODE) {
        
        // If direct...
        if (logical_block < POINTERS_PER_INODE) {

            //Check if block exists and if not, allocate block
            if(block.inode[ioffset].direct[logical_block] == 0){
                //search bitmap, update free list, update inode, write inode change to disk
                found = 0;
                for(int i = 0; i < block.super.nblocks; i++){
                    if(bitmap[i] == 0){
                        found = 1;
                        bitmap[i] = 1;
                        block.inode[ioffset].direct[logical_block] = i;
                        disk_write(thedisk,iblock, block.data);
                        break;
                    }
                }
                if(!found) break;
            }

            // Write to direct block
            union fs_block direct_block;

            // Copy to data buffer
            memcpy(direct_block.data + next_block_offset, data + bytes_written, to_write);       
            
            //once copied, can write into direct block
            disk_write(thedisk, block.inode[ioffset].direct[logical_block], direct_block.data);
            bytes_written += to_write;
        }

        // If indirect...
        else {

            //Check if indirect block exists
            //if not, we allocate block
            if(block.inode[ioffset].indirect == 0){
                found = 0;
                for(int i = 0; i < block.super.nblocks; i++){
                    if(bitmap[i] == 0){
                        found = 1;
                        bitmap[i] = 1;
                        block.inode[ioffset].indirect = i;
                        disk_write(thedisk, iblock, block.data);
                        break;
                    }
                }
                if(found == 0) break;
            }

            // Read indirect block
            union fs_block indirect_block;
            if(block.inode[ioffset].indirect != 0){
                disk_read(thedisk, block.inode[ioffset].indirect, indirect_block.data);
                for(int i = 0; i < POINTERS_PER_BLOCK; i++){
                    if(indirect_block.pointers[i] > 1000){
                        indirect_block.pointers[i] = 0;
                    }
                }
            } else {
                for(int i = 0; i < POINTERS_PER_BLOCK; i++){
                    indirect_block.pointers[i] = 0;
                }
            }

            //Check if indirect pointers exist
            if(indirect_block.pointers[logical_block - POINTERS_PER_INODE] == 0){
                //search bitmap, update free list, update inode, write inode change to disk
                found = 0;
                for(int i = 0; i < block.super.nblocks; i++){
                    if(bitmap[i] == 0){
                        found = 1;
                        bitmap[i] = 1;
                        indirect_block.pointers[logical_block - POINTERS_PER_INODE] = i;
                        disk_write(thedisk,block.inode[ioffset].indirect,indirect_block.data);
                        break;
                    }
                }
                if(found == 0) break;
            }

            // Read pointers
            union fs_block next_block;

            // Copy to data buffer
            memcpy(next_block.data + next_block_offset, data + bytes_written, to_write);
            
            // Write next block
            disk_write(thedisk, indirect_block.pointers[logical_block - POINTERS_PER_INODE], next_block.data);

            bytes_written += to_write;
        }

        //Update inode size
        if(offset != 0){
            block.inode[ioffset].size += to_write;
            disk_write(thedisk,iblock,block.data);
        } else {
            if(block.inode[ioffset].size > offset){
                block.inode[ioffset].size = block.inode[ioffset].size + to_write;
            } else {
                block.inode[ioffset].size = offset + to_write;
            }
            disk_write(thedisk, iblock,block.data);
        }

        //Update to_write
        if(length - bytes_written > BLOCK_SIZE){
            to_write = BLOCK_SIZE;
        } else {
            to_write = length - bytes_written;
        }
        
        //Set next_block_offset to 0
        next_block_offset = 0;

        logical_block += 1;

    }
    
	return bytes_written;
}