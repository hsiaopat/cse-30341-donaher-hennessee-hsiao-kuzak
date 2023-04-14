/* main.c : Main file for redextract */

#include <stdio.h>
#include <stdlib.h>

/* for strdup due to C99 */
char * strdup(const char *s);

#include <string.h>
#include <pthread.h>

#include "pcap-read.h"
#include "pcap-process.h"
#include "packet.h"

pthread_mutex_t StackLock;
pthread_mutex_t TableLock;

pthread_cond_t PushCond;
pthread_cond_t PopCond;

int StackSize = 0;
int AllDone = 0;
char KeepGoing = 1;
struct Packet * StackItems[STACK_MAX_SIZE];

void * thread_producer (void * pData) 
{
    struct FilePcapInfo * theInfo = (struct FilePcapInfo *) pData;

    /* Read file and push packets */
    readPcapFile(theInfo);

    return NULL;
}

void * thread_consumer (void * pData)
{
    struct Packet * pPacket;

    while (KeepGoing) {

        pthread_mutex_lock(&StackLock);

        /* No packets to pop, must wait :( */
        while (StackSize <= 0) {
            
            pthread_cond_wait(&PopCond, &StackLock);

            /* Ope, we're done! */
            if (StackSize == 0 && AllDone) {
                pthread_mutex_unlock(&StackLock);
                return NULL;
            }
        }

        /* Pop packet */
        pPacket = StackItems[StackSize-1];
        StackSize--;

        /* Tell producers there is room to push */
        pthread_cond_signal(&PushCond);

        pthread_mutex_unlock(&StackLock);

        /* Process packcet */
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
    theInfo.MaxPackets = 0;

    /* Producer */
    pthread_t pThreadProducer;

    /* Consumers */
    int     nThreadsConsumers = 1;
    pthread_t * pThreadConsumers;
    pThreadConsumers = (pthread_t *) malloc(sizeof(pthread_t *) * nThreadsConsumers);

    /* Start up producer thread */
    pthread_create(&pThreadProducer, 0, thread_producer, &theInfo);

    /* Start up consumer threads */
    for (int i = 0; i < nThreadsConsumers; i++) {
        pthread_create(&pThreadConsumers[i], 0, thread_consumer, 0);
    }

    /* Wait for producer to finish */
    pthread_join(pThreadProducer, 0);

    /* Signal to consumers that producers are done */
    AllDone = 1;
    pthread_cond_broadcast(&PopCond);

    /* Loop until the consumers are done */
    for (int i = 0; i < nThreadsConsumers; i++) {
        pthread_join(pThreadConsumers[i], 0);
    }
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

    /* Locks */
    pthread_mutex_init(&StackLock, 0);
    pthread_mutex_init(&TableLock, 0);

    printf("MAIN: Initializing the table for redundancy extraction\n");
    initializeProcessing(DEFAULT_TABLE_SIZE);
    printf("MAIN: Initializing the table for redundancy extraction ... done\n");

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

    return 0;
}