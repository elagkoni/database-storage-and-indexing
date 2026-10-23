#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include "secondaryinfo.h"
#include "hashtable.h"
#include "files.h"
#include "utilities.h"

SecondaryInfo::SecondaryInfo(char* originalName, const char* attrName, int attrLength, int bucketCount, char* fileName)
    : AbstractHashtable(originalName, attrName, attrLength, bucketCount)
{
    filetype = SECONDARY_FILETYPE;
    this->fileName = utilities::strdup(fileName);
    this->fileNameLength = strlen(fileName);
}

SecondaryInfo::SecondaryInfo(SHT_info info)
    : AbstractHashtable(info.originalName, info.attrName, info.attrLength, info.bucketCount)
{
    filetype = SECONDARY_FILETYPE;
    fileName = utilities::strdup(info.fileName);
    fileNameLength = strlen(info.fileName);
}

SecondaryInfo::SecondaryInfo(char* block)
    : AbstractHashtable(block)
{
    block += AbstractHashtable::sizeInBytes();
    std::memcpy(&fileNameLength, block, sizeof(fileNameLength));
    block += sizeof(fileNameLength);
    fileName = new char[fileNameLength + 1];
    std::memcpy(fileName, block, sizeof(char) * fileNameLength + 1);
}

SecondaryInfo::~SecondaryInfo()
{
    delete[] fileName;
}

SecondaryInfo* SecondaryInfo::createFromBlock(void* block)
{
    char* address = static_cast<char*>(block);
    FILETYPE type = INVALID_FILETYPE;
    std::memcpy(&type, address, sizeof(filetype));
    return type == SECONDARY_FILETYPE ? new SecondaryInfo(address) : nullptr;
}

void SecondaryInfo::saveToBlock(void* block) const
{
    char* address = static_cast<char*>(block);
    AbstractHashtable::saveToBlock(block);
    address += AbstractHashtable::sizeInBytes();
    std::memcpy(address, &fileNameLength, sizeof(fileNameLength));
    address += sizeof(fileNameLength);
    std::memcpy(address, block, sizeof(char) * fileNameLength + 1);
}

/// Sedgewick's Algorithms in C, Parts 1-4, 3rd American Edition.
int SecondaryInfo::hash(const char* key) const
{
    int h = 0, a = 127;
    for(; *key != '\0'; key++)
        h = (a * h + *key) % bucketCount;
    return h;
}
