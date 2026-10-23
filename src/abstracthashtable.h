#ifndef ABSTRACTHASHTABLE_H
#define ABSTRACTHASHTABLE_H

#include <vector>
#include "abstractfile.h"
#include "files.h"

class AbstractHashtable : public AbstractFile
{
public:
    AbstractHashtable(char* originalName, const char* attrName, int attrLength, int bucketCount);
    AbstractHashtable(char* block);
    ~AbstractHashtable();

    /// Block size = 512 bytes
    virtual void saveToBlock(void* block) const override = 0;

    std::pair<int, int> blockIdToBucketAndBlock(int blockId) const;
    int bucketAndBlockToBlockId(int bucket, int bucketBlockId) const;

    char* originalName;
    int originalNameLength;
    int bucketCount;

protected:
    size_t sizeInBytes() const;
};

#endif // ABSTRACTHASHTABLE_H
