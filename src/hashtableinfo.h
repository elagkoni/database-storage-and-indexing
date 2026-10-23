#ifndef HASHTABLEINFO_H
#define HASHTABLEINFO_H

#include "abstracthashtable.h"
#include "hashtable.h"
#include "files.h"


// Used to group internal HashTable functions
class HashTableInfo : public AbstractHashtable
{
public:
    HashTableInfo(char* originalName, char attrType, const char* attrName, int attrLength, int bucketCount); // used in HT_CreateIndex
    HashTableInfo(HT_info info);
    HashTableInfo(char* block); // used in HashTableInfo::createFromBlock

    /// Block size = 512 bytes
    static HashTableInfo* createFromBlock(void* block);
    void saveToBlock(void* block) const override;

    int hash(const int key) const;

    char attrType;      // key type (string or integer)
};
#endif // HASHTABLEINFO_H
