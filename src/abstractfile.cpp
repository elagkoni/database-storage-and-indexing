#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include "abstractfile.h"
#include "utilities.h"


AbstractFile::AbstractFile(const char *attrName, int attrLength)
{
    this->filetype = INVALID_FILETYPE;
    this->attrName = utilities::strdup(attrName);
    this->attrLength = attrLength;
}

AbstractFile::AbstractFile(char *block)
{
    std::memcpy(&filetype, block, sizeof(filetype));
    block += sizeof(filetype);
    std::memcpy(&attrLength, block, sizeof(attrLength));
    block += sizeof(attrLength);
    attrName = new char[attrLength + 1];
    std::memcpy(attrName, block, sizeof(char) * attrLength + 1);
}

void AbstractFile::saveToBlock(void *block) const
{
    char* address = static_cast<char*>(block);
    std::memcpy(address, &filetype, sizeof(filetype));
    address += sizeof(filetype);
    std::memcpy(address, &attrLength, sizeof(attrLength));
    address += sizeof(attrLength);
    std::memcpy(address, attrName, sizeof(char) * attrLength + 1);
}

size_t AbstractFile::sizeInBytes() const
{
    return sizeof(filetype) + sizeof(attrLength) + (sizeof(char) * attrLength + 1);
}
