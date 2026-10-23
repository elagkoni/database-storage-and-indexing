#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "record.h"
#include "heapfile.h"

typedef struct {
    char* originalName; // used to name heapfiles
    int originalNameLength;
    int fd;
    char attrType;
    char* attrName;
    int attrLength;
    int bucketCount;
} HT_info;

int HT_CreateIndex(char* fileName, char attrType, char* attrName, int attrLength, int bucketCount);
HT_info* HT_OpenIndex(char* fileName);
int HT_CloseIndex(HT_info* headerInfo);
int HT_InsertEntry(HT_info headerInfo, Record record);
int HT_DeleteEntry(HT_info headerInfo, void* value);
int HT_GetAllEntries(HT_info headerInfo, void* value);
int HT_GetAllEntriesAtBucketAndBlock(HT_info headerInfo, void* value, int bucket, int block);

#endif // HASHTABLE_H
