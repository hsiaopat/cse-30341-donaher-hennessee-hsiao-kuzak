/* main.c : Main file for redextract */

#include <stdio.h>
#include <stdlib.h>

/* for strdup due to C99 */
char * strdup(const char *s);

#include <string.h>

#include "pcap-read.h"
#include "pcap-process.h"
#include "packet.h"
#include <pthread.h>

pthread_mutex_t StackLock;
pthread_mutex_t TableLock;

pthread_cond_t PushCond;
pthread_cond_t PopCond;

#define STACK_MAX_SIZE 1000 // FIXME
struct Packet * StackItems[STACK_MAX_SIZE];

int StackSize = 0;
//char KeepGoing = 1;

struct ThreadDataProduce
{
    struct FilePcapInfo * theInfo;
    int ThreadID;
};

void * thread_producer (void * pData) 
{   
    // /* unpack arguments */
    // struct ThreadDataProduce * pThreadData;
    // pThreadData = (struct ThreadDataProduce *) pData;

    // printf("hi\n");
    // printf("%s\n", pThreadData->theInfo->FileName);

    struct FilePcapInfo * theInfo = (struct FilePcapInfo *) pData;

    //printf("%s\n", theInfo->FileName);

    /* read file and push packets */
    readPcapFile(theInfo);

    return NULL;
}

void * thread_consumer (void * pData)
{
    char KeepGoing = 1;
    struct Packet * pPacket;

    while (KeepGoing) {

        pthread_mutex_lock(&StackLock);

        if (StackSize <= 0) {
            pthread_cond_broadcast(&PopCond);
            pthread_mutex_unlock(&StackLock);
            return NULL;
        }

        /* pop */
        pPacket = StackItems[StackSize-1];
        StackSize--;

        //pthread_cond_signal(&PushCond);
        //pthread_cond_signal(&PopCond);

        pthread_mutex_unlock(&StackLock);

        /* process */
        pthread_mutex_lock(&TableLock);
        processPacket(pPacket);
        pthread_mutex_unlock(&TableLock);
    }
    
    return NULL;
}

void processPcapFile(char * theFile) 
{
    struct FilePcapInfo theInfo;

    theInfo.FileName = theFile;
    theInfo.EndianFlip = 0;
    theInfo.BytesRead = 0;
    theInfo.Packets = 0;
    theInfo.MaxPackets = 5;

    //int     nThreadsProducers = 1;
    int     nThreadsConsumers = 3;

    pthread_t pThreadProducer;
    pthread_t *     pThreadConsumers;

    /* Allocate space for tracking the threads */
    //pThreadProducers = (pthread_t *) malloc(sizeof(pthread_t *) * nThreadsProducers); 
    pThreadConsumers = (pthread_t *) malloc(sizeof(pthread_t *) * nThreadsConsumers);

    /* Start up our producer threads */
    //for (int i = 0; i < nThreadsProducers; i++) {

    // struct ThreadDataProduce * pThreadData;

    // pThreadData = (struct ThreadDataProduce *) malloc(sizeof(struct ThreadDataProduce));
    
    // pThreadData->theInfo = &theInfo;
    // pThreadData->ThreadID = i;

    pthread_create(&pThreadProducer, 0, thread_producer, &theInfo);
    pthread_join(pThreadProducer, 0);
    //}

    //printf("%d\n", StackSize);

    // for (int i = 0; i < nThreadsProducers; i++) {
    //     pthread_join(pThreadProducers[i], 0);
    // }

    // while (StackSize > 0) {
    //     int tasksLeft = nThreadsConsumers;
    //     if (StackSize - tasksLeft < 0) {
    //         tasksLeft = StackSize;
    //     }

    /* Start up our consumer threads */
    for (int i = 0; i < nThreadsConsumers; i++) {
        pthread_create(&pThreadConsumers[i], 0, thread_consumer, 0);
    }

    /* Loop until the consumers are done */
    for (int i = 0; i < nThreadsConsumers; i++) {
        pthread_join(pThreadConsumers[i], 0);
    }
    // }
}

int main (int argc, char *argv[])
{ 
    if (argc < 2)
    {
        printf("Usage: redextract FileX\n");
        printf("       redextract FileX\n");
        printf("  FileList        List of pcap files to process\n");
        printf("    or\n");
        printf("  FileName        Single file to process (if ending with .pcap)\n");
        printf("\n");
        printf("Optional Arguments:\n");
        /* You should handle this argument but make this a lower priority when 
           writing this code to handle this 
         */
        printf("  -threads N       Number of threads to use (2 to 8)\n");
        /* Note that you do not need to handle this argument in your code */
        printf("  -window  W       Window of bytes for partial matching (64 to 512)\n");
        printf("       If not specified, the optimal setting will be used\n");
        return -1;
    }

    printf("MAIN: Initializing the table for redundancy extraction\n");
    initializeProcessing(DEFAULT_TABLE_SIZE);
    printf("MAIN: Initializing the table for redundancy extraction ... done\n");

    pthread_mutex_init(&StackLock, 0);
    pthread_mutex_init(&TableLock, 0);

    // if ends in pcap
        // call processPcapFile(argv[1])

    // else
    FILE *fp;
    char buffer[100];

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Unable to open file\n");
        return 1;
    }

    /* read pcap files */
    while(fgets(buffer, 100, fp)) {

        /* remove trailing newline */
        int len = strlen(buffer);
        buffer[len-1] = (buffer[len-1] == '\n') ? '\0' : buffer[len-1];

        /* execute producer/consumer code */
        processPcapFile(buffer);
    }

    fclose(fp);

    printf("Summarizing the processed entries\n");
    tallyProcessing();

    /* Output the statistics */

    printf("Parsing of file %s complete\n", argv[1]);

    printf("  Total Packets Parsed:    %d\n", gPacketSeenCount);
    printf("  Total Bytes   Parsed:    %lu\n", (unsigned long) gPacketSeenBytes);
    printf("  Total Packets Duplicate: %d\n", gPacketHitCount);
    printf("  Total Bytes   Duplicate: %lu\n", (unsigned long) gPacketHitBytes);

    float fPct;

    fPct = (float) gPacketHitBytes / (float) gPacketSeenBytes * 100.0;

    printf("  Total Duplicate Percent: %6.2f%%\n", fPct);

    //getchar();

    return 0;
}