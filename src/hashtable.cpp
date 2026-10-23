#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include "hashtable.h"
#include "hashtableinfo.h"
#include "heapfile.h"
#include "heapfileinfo.h"
#include "files.h"
#include "utilities.h"
extern "C" {
    #include "../lib/BF_custom.h"
}

int HT_CreateIndex(char* fileName, char attrType, char* attrName, int attrLength, int bucketCount)
{
    // Create file and allocate block
    int r = BF_CreateFile(fileName);
    if (r < 0) {
        return -1;
    }
    int fd = BF_OpenFile(fileName);
    if (fd < 0) {
        return -1;
    }
    r = BF_AllocateBlock(fd);
    if (r < 0) {
        return -1;
    }
    // Save HT_info in block_0
    HashTableInfo htInfo(fileName, attrType, attrName, attrLength, bucketCount);
    void* block = nullptr;
    r = BF_ReadBlock(fd, 0, &block);
    if (r < 0) {
        return -1;
    }
    htInfo.saveToBlock(block);
    r = BF_WriteBlock(fd, 0);
    if (r < 0) {
        return -1;
    }
    // Close file
    BF_CloseFile(r);
    if (r < 0) {
        return -1;
    }
    return 0;
}

HT_info* HT_OpenIndex(char* fileName)
{
    int fd = BF_OpenFile(fileName);
    if (fd < 0) {
        return nullptr;
    }

    void* block = nullptr;
    int r = BF_ReadBlock(fd, 0, &block);
    if (r < 0) {
        return nullptr;
    }

    std::unique_ptr<HashTableInfo> hinfo(HashTableInfo::createFromBlock(block));
    if (!hinfo) {
        return nullptr;
    }

    HT_info* cinfo = new HT_info {
                    .originalName = strdup(hinfo->originalName),
                    .originalNameLength = hinfo->originalNameLength,
                    .fd = fd,
                    .attrType = hinfo->attrType,
                    .attrName = strdup(hinfo->attrName),
                    .attrLength = hinfo->attrLength,
                    .bucketCount = hinfo->bucketCount
                    };

    return cinfo;
}

int HT_CloseIndex(HT_info* headerInfo)
{
    int r = BF_CloseFile(headerInfo->fd);
    if (r < 0) {
        return -1;
    }
    free(headerInfo->attrName);
    free(headerInfo->originalName);
    delete headerInfo;
    return 0;
}

int HT_InsertEntry(HT_info headerInfo, Record record)
{
    HashTableInfo ht(headerInfo);
    int hashedKey = ht.hash(record.id);
#ifdef TESTING
    std::cout << "Inserting record " << record.id << " at: " << hashedKey << std::endl;
#endif
    //
    // Open the heapfile corresponding to the bucket
    //
    std::string s(headerInfo.originalName + std::to_string(hashedKey));
    HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));
    if (!hp) { // if there is no such heapfile create it
        std::string s(headerInfo.originalName + std::to_string(hashedKey));
        int r = HP_CreateFile(const_cast<char*>(s.c_str()), headerInfo.attrType, headerInfo.attrName, headerInfo.attrLength);
        if (r < 0) {
            return -1;
        }
        hp = HP_OpenFile(const_cast<char*>(s.c_str()));
    }
    //
    // Insert entry to heapfile/bucket and close heapfile
    //
    int r = HP_InsertEntry(*hp, record);
    if (r > 0) {
        r += r * ht.bucketCount + hashedKey;
    }
    HP_CloseFile(hp);
    return r;
}

int HT_DeleteEntry(HT_info headerInfo, void* value)
{
    HashTableInfo ht(headerInfo);
    int hashedKey = ht.hash(*static_cast<int*>(value));
#ifdef TESTING
    std::cout << "Deleting record " << *((int*)value) << " at: " << hashedKey << std::endl;
#endif
    //
    // Open the heapfile corresponding to the bucket
    //
    std::string s(headerInfo.originalName + std::to_string(hashedKey));
    HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));
    if (!hp) {
        return -1;
    }
    //
    // Delete entry from heapfile/bucket and close heapfile
    //
    int r = HP_DeleteEntry(*hp, value);
    HP_CloseFile(hp);
    return r;
}

int HT_GetAllEntries(HT_info headerInfo, void* value)
{
    HashTableInfo ht(headerInfo);
    int hashedKey = ht.hash(*static_cast<int*>(value));
#ifdef TESTING
    std::cout << "Getting record " << *((int*)value) << " at: " << hashedKey << std::endl;
#endif
    //
    // Open the heapfile corresponding to the bucket
    //
    std::string s(headerInfo.originalName + std::to_string(hashedKey));
    HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));
    if (!hp) {
        return -1;
    }
    //
    // Search for entry in the heapfile/bucket and close the heapfile
    //
    int r = HP_GetAllEntries(*hp, value);
    HP_CloseFile(hp);
    return r;
}

int HT_GetAllEntriesAtBucketAndBlock(HT_info headerInfo, void *value, int bucket, int block)
{
    HashTableInfo ht(headerInfo);
    int hashedKey = bucket;
#ifdef TESTING
    std::cout << "Getting record " << *((int*)value) << " at: " << hashedKey << std::endl;
#endif
    //
    // Open the heapfile corresponding to the bucket
    //
    std::string s(headerInfo.originalName + std::to_string(hashedKey));
    HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));
    if (!hp) {
        return -1;
    }
    //
    // Search for entry in the heapfile/bucket and close the heapfile
    //
    int r = HP_GetAllEntriesAtBlock(*hp, value, block);
    HP_CloseFile(hp);
    return r;
}
