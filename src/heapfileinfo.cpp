#include <iostream>
#include <cstring>
#include "heapfileinfo.h"
#include "utilities.h"
#include "record.h"
extern "C" {
    #include "../lib/BF_custom.h"
}

HeapFileInfo::HeapFileInfo(char attrType, const char* attrName, int attrLength, int totalRecords, int nextBlock)
    : AbstractFile(attrName, attrLength)
{
    filetype = HEAP_FILETYPE;
    this->attrType = attrType;
    this->totalRecords = totalRecords;
    this->nextBlock = nextBlock;
}

HeapFileInfo::HeapFileInfo(HP_info info)
    : AbstractFile(info.attrName, info.attrLength)
{
    filetype = HEAP_FILETYPE;
    attrType = info.attrType;
    totalRecords = info.totalRecords;
    nextBlock = info.nextBlock;
}

HeapFileInfo::HeapFileInfo(char* block)
    : AbstractFile(block)
{
    block += AbstractFile::sizeInBytes();
    std::memcpy(&attrType, block, sizeof(attrType));
    block += sizeof(attrType);
    std::memcpy(&totalRecords, block, sizeof(totalRecords));
    block += sizeof(totalRecords);
    std::memcpy(&nextBlock, block, sizeof(nextBlock));
}

HeapFileInfo* HeapFileInfo::createFromBlock(void* block)
{
    char* address = static_cast<char*>(block);
    FILETYPE type = INVALID_FILETYPE;
    std::memcpy(&type, address, sizeof(filetype));
    return type == HEAP_FILETYPE ? new HeapFileInfo(address) : nullptr;
}

void HeapFileInfo::saveToBlock(void* block) const
{
    char* address = static_cast<char*>(block);
    AbstractFile::saveToBlock(block);
    address += AbstractFile::sizeInBytes();
    std::memcpy(address, &attrType, sizeof(attrType));
    address += sizeof(attrType);
    std::memcpy(address, &totalRecords, sizeof(totalRecords));
    address += sizeof(totalRecords);
    std::memcpy(address, &nextBlock, sizeof(nextBlock));
}

int HeapFileInfo::blockCount() const
{
    return totalRecords % MAX_RECORDS == 0 ? totalRecords / MAX_RECORDS + 1 : totalRecords / MAX_RECORDS + 2;
}
