#ifndef SECONDARYINFO_H
#define SECONDARYINFO_H

#include "abstracthashtable.h"
#include "secondary.h"
#include "files.h"

// Used to group internal SecondaryHashTable functions
class SecondaryInfo : public AbstractHashtable
{
public:
    SecondaryInfo(char* originalName, const char* attrName, int attrLength, int bucketCount, char* fileName); // used in SHT_CreateIndex
    SecondaryInfo(SHT_info info);
    SecondaryInfo(char* block); // used in SecondaryInfo::createFromBlock
    ~SecondaryInfo();

    /// Block size = 512 bytes
    static SecondaryInfo* createFromBlock(void* block);
    void saveToBlock(void* block) const override;

    int hash(const char* key) const;

    char* fileName;
    int fileNameLength;
};

#endif // SECONDARYINFO_H
