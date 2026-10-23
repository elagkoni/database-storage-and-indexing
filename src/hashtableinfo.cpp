#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include "hashtableinfo.h"
#include "hashtable.h"
#include "heapfile.h"
#include "heapfileinfo.h"
#include "files.h"
#include "utilities.h"


HashTableInfo::HashTableInfo(char *originalName, char attrType, const char *attrName, int attrLength, int bucketCount)
    : AbstractHashtable(originalName, attrName, attrLength, bucketCount)
{
    this->filetype = HASHTABLE_FILETYPE;
    this->attrType = attrType;
}

HashTableInfo::HashTableInfo(HT_info info)
    : AbstractHashtable(info.originalName, info.attrName, info.attrLength, info.bucketCount)
{
    filetype = HASHTABLE_FILETYPE;
    attrType = info.attrType;
}

HashTableInfo::HashTableInfo(char* block)
    : AbstractHashtable(block)
{
    block += AbstractHashtable::sizeInBytes();
    std::memcpy(&attrType, block, sizeof(attrType));
}

HashTableInfo *HashTableInfo::createFromBlock(void* block)
{
    char* address = static_cast<char*>(block);
    FILETYPE type = INVALID_FILETYPE;
    std::memcpy(&type, address, sizeof(filetype));
    return type == HASHTABLE_FILETYPE ? new HashTableInfo(address) : nullptr;
}

void HashTableInfo::saveToBlock(void* block) const
{
    char* address = static_cast<char*>(block);
    AbstractHashtable::saveToBlock(block);
    address += AbstractHashtable::sizeInBytes();
    std::memcpy(address, &attrType, sizeof(attrType));
}

int HashTableInfo::hash(const int key) const
{
    return key % bucketCount;
}
