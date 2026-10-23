#ifndef HEAPFILEINFO_H
#define HEAPFILEINFO_H

#include "abstractfile.h"
#include "files.h"
#include "heapfile.h"
extern "C" {
      #include "../lib/BF_custom.h"
}
#include "record.h"

// Used to group internal Heapfile functions
class HeapFileInfo : public AbstractFile
{
public:
    HeapFileInfo(char attrType, const char* attrName, int attrLength, int totalRecords, int nextBlock);
    HeapFileInfo(HP_info info);
    HeapFileInfo(char* block);
    virtual ~HeapFileInfo() = default;

    static HeapFileInfo* createFromBlock(void* block);

    // Block size = 512 bytes
    void saveToBlock(void* block) const;
    int blockCount() const;

    char attrType;      // key type (string or integer)
    int totalRecords;
    int nextBlock;
};

#endif // HEAPFILEINFO_H
