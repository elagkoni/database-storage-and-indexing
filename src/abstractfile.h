#ifndef ABSTRACTFILE_H
#define ABSTRACTFILE_H

#include <vector>
#include "files.h"

class AbstractFile
{
public:
    AbstractFile(const char* attrName, int attrLength);
    AbstractFile(char* block);
    virtual ~AbstractFile() = default; 

    /// Block size = 512 bytes
    virtual void saveToBlock(void* block) const = 0;

    FILETYPE filetype;
    int attrLength;     // key string length
    char* attrName;     // key string

protected:
    size_t sizeInBytes() const;
};

#endif // ABSTRACTFILE_H
