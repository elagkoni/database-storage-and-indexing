#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include "abstracthashtable.h"
#include "utilities.h"


AbstractHashtable::AbstractHashtable(char* originalName, const char* attrName, int attrLength, int bucketCount)
    : AbstractFile(attrName, attrLength)
{
    this->originalName = utilities::strdup(originalName);
    this->originalNameLength = strlen(originalName);
    this->bucketCount = bucketCount;
}

AbstractHashtable::AbstractHashtable(char* block)
    : AbstractFile(block)
{
    block += AbstractFile::sizeInBytes();
    std::memcpy(&originalNameLength, block, sizeof(originalNameLength));
    block += sizeof(originalNameLength);
    originalName = new char[originalNameLength + 1];
    std::memcpy(originalName, block, sizeof(char) * originalNameLength + 1);
    block += sizeof(char) * originalNameLength + 1;
    std::memcpy(&bucketCount, block, sizeof(bucketCount));
}

AbstractHashtable::~AbstractHashtable()
{
    delete[] originalName;
}

void AbstractHashtable::saveToBlock(void* block) const
{
    char* address = static_cast<char*>(block);
    AbstractFile::saveToBlock(block);
    address += AbstractFile::sizeInBytes();
    std::memcpy(address, &originalNameLength, sizeof(originalNameLength));
    address += sizeof(originalNameLength);
    std::memcpy(address, originalName, sizeof(char) * originalNameLength + 1);
    address += sizeof(char) * originalNameLength + 1;
    std::memcpy(address, &bucketCount, sizeof(bucketCount));
}

std::pair<int, int> AbstractHashtable::blockIdToBucketAndBlock(int blockId) const
{
    blockId = blockId - 1 - bucketCount;
    return std::pair<int, int>(blockId % bucketCount, blockId / bucketCount);
}

int AbstractHashtable::bucketAndBlockToBlockId(int bucket, int bucketBlockId) const
{
    return bucketCount + 1 + bucket + bucketBlockId * bucketCount;
}

size_t AbstractHashtable::sizeInBytes() const
{
    return AbstractFile::sizeInBytes() + sizeof(originalNameLength)
            + (sizeof(char) * originalNameLength + 1) + sizeof(bucketCount);
}
