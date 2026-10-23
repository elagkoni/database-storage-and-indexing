#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include "hashtable.h"
#include "heapfile.h"
#include "hashtableinfo.h"
#include "secondary.h"
#include "secondaryinfo.h"
#include "files.h"
#include "utilities.h"
extern "C" {
    #include "../lib/BF_custom.h"
}

int SHT_CreateSecondaryIndex(char* sfileName, char* attrName, int attrLength, int bucketCount, char* fileName)
{
    // Create file and allocate block
    int r = BF_CreateFile(sfileName);
    if (r < 0) {
        return -1;
    }
    int fd = BF_OpenFile(sfileName);
    if (fd < 0) {
        return -1;
    }
    r = BF_AllocateBlock(fd);
    if (r < 0) {
        return -1;
    }
    // Save SHT_info in block_0
    SecondaryInfo shtInfo(sfileName, attrName, attrLength, bucketCount, fileName);
    void* block = nullptr;
    r = BF_ReadBlock(fd, 0, &block);
    if (r < 0) {
        return -1;
    }
    shtInfo.saveToBlock(block);
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

SHT_info* SHT_OpenSecondaryIndex(char* sfileName)
{
    int fd = BF_OpenFile(sfileName);
    if (fd < 0) {
        return nullptr;
    }

    void* block = nullptr;
    int r = BF_ReadBlock(fd, 0, &block);
    if (r < 0) {
        return nullptr;
    }

    std::unique_ptr<SecondaryInfo> shinfo(SecondaryInfo::createFromBlock(block));
    if (!shinfo) {
        return nullptr;
    }

    SHT_info* cinfo = new SHT_info {
        .originalName = strdup(shinfo->originalName),
        .originalNameLength = shinfo->originalNameLength,
        .fileDesc = fd,
        .attrName = strdup(shinfo->attrName),
        .attrLength = shinfo->attrLength,
        .bucketCount = shinfo->bucketCount,
        .fileName = strdup(shinfo->fileName),
        .fileNameLength = shinfo->fileNameLength
    };

    return cinfo;
}

int SHT_CloseSecondaryIndex(SHT_info* headerInfo)
{
    int r = BF_CloseFile(headerInfo->fileDesc);
    if (r < 0) {
        return -1;
    }
    free(headerInfo->originalName);
    free(headerInfo->attrName);
    free(headerInfo->fileName);
    delete headerInfo;
    return 0;
}

int SHT_SecondaryInsertEntry(SHT_info headerInfo, SecondaryRecord record)
{
    SecondaryInfo sht(headerInfo);
    int hashedKey = sht.hash(record.record.surname);
#ifdef TESTING
    std::cout << "Inserting " << record.record.surname << " at: " << hashedKey << std::endl;
#endif
    //
    // Open the heapfile corresponding to the bucket
    //
    std::string s(headerInfo.originalName + std::to_string(hashedKey));
    HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));

    if (!hp) { // if there is no such heapfile create it
        std::string s(headerInfo.originalName + std::to_string(hashedKey));
        int r = HP_CreateFile(const_cast<char*>(s.c_str()), 'c', headerInfo.attrName, headerInfo.attrLength);
        if (r < 0) {
            return -1;
        }
        hp = HP_OpenFile(const_cast<char*>(s.c_str()));
    }
    //
    // Insert entry to heapfile/bucket and close heapfile
    //
    SecondaryRecordCustom srec;
    srec.blockId = record.blockId;
    std::strncpy(srec.surname, record.record.surname, 25);
    int r = HP_SecondaryInsertEntry(*hp, srec);
    HP_CloseFile(hp);
    return r >= 0 ? 0 : -1;
}

int SHT_SecondaryGetAllEntries(SHT_info headerInfoSht, HT_info headerInfoHt, void* value)
{
    HashTableInfo ht(headerInfoHt);
    SecondaryInfo sht(headerInfoSht);
    int hashedKey = sht.hash(static_cast<char*>(value));
#ifdef TESTING
    std::cout << "Getting " << static_cast<char*>(value) << " at: " << hashedKey << std::endl;
#endif
    //
    // Open the heapfile corresponding to the bucket
    //
    std::string s(headerInfoSht.originalName + std::to_string(hashedKey));
    HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));
    if (!hp) {
        return -1;
    }
    //
    // Search for entry in the heapfile/bucket and close the heapfile
    //
    std::vector<std::pair<int, int>> blockIds = HP_SecondaryGetBlocks(*hp, value); // (blockId, count) count is used for special blocks
    if (blockIds.at(0).first == -1) { // either encountered an error or didn't find the element
        HP_CloseFile(hp);
        return -1;
    }
    int blocksRead = 0;
    for (auto& x : blockIds) {
        if (x.first == -1) {
            blocksRead = x.second;
            break;
        }
        std::pair<int, int> pair = ht.blockIdToBucketAndBlock(x.first);
        HT_GetAllEntriesAtBucketAndBlock(headerInfoHt, value, pair.first, pair.second); // does not need count, prints all occurences in the block
        blocksRead++; // one block read from HT_GetAllEntriesAtBucketAndBlock
    }
    HP_CloseFile(hp);
    return blocksRead;
}
