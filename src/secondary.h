#ifndef SECONDARY_H
#define SECONDARY_H
#include "record.h"
#include "hashtable.h"

typedef struct {
    char* originalName; // used to name heapfiles
    int originalNameLength;
    int fileDesc;
    char* attrName;
    int attrLength;
    long int bucketCount;
    char* fileName;
    int fileNameLength;
} SHT_info;

int SHT_CreateSecondaryIndex(char* sfileName, char* attrName, int attrLength, int bucketCount, char* fileName);
SHT_info* SHT_OpenSecondaryIndex(char* sfileName);
int SHT_CloseSecondaryIndex(SHT_info* headerInfo);
int SHT_SecondaryInsertEntry(SHT_info headerInfo, SecondaryRecord record);
int SHT_SecondaryGetAllEntries(SHT_info headerInfoSht, HT_info headerInfoHt, void* value);

#endif // SECONDARY_H
