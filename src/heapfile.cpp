#include <iostream>
#include <memory>
#include <cstring>
#include <memory>
#include <algorithm>
#include "heapfile.h"
#include "heapfileinfo.h"
#include "files.h"
extern "C" {
    #include "../lib/BF_custom.h"
}

int HP_CreateFile(char* fileName, char attrType, char* attrName, int attrLength)
{
    // Create file and allocate block
    int r = BF_CreateFile(fileName);
    if (r < 0) {
        return -1;
    }

    int fd = BF_OpenFile(fileName);
    if (fd < 0) {
        return -1;
    }

    r = BF_AllocateBlock(fd);
    if (r < 0) {
        return -1;
    }

    // Insert heap info into block_0
    int total=0, next=0;
    HeapFileInfo hfInfo(attrType, attrName, attrLength, total, next);
    void* block = nullptr;

    r = BF_ReadBlock(fd, 0, &block);
    if (r < 0) {
        return -1;
    }
    hfInfo.saveToBlock(block);
    r = BF_WriteBlock(fd, 0);
    if (r < 0) {
        return -1;
    }

    // Close file and return
    r = BF_CloseFile(fd);
    if (r < 0) {
        return -1;
    }
    return 0;
}

HP_info* HP_OpenFile(char* fileName)
{
    //
    // open file
    //
    int fd = BF_OpenFile(fileName);
    if (fd < 0) {
        return nullptr;
    }
    void* block = nullptr;
    int r = BF_ReadBlock(fd, 0, &block);
    if (r < 0) {
        return nullptr;
    }
    //
    // check if it's a heap file
    //
    HeapFileInfo* hinfo = HeapFileInfo::createFromBlock(block);
    if (!hinfo) {
        return nullptr;
    }

    HP_info* cinfo = new HP_info {
        .fileDesc = fd,
        .attrType = hinfo->attrType,
        .attrName = strdup(hinfo->attrName),
        .attrLength = hinfo->attrLength,
        .totalRecords = hinfo->totalRecords,
        .nextBlock = hinfo->nextBlock
    };

    delete hinfo;
    return cinfo;
}

int HP_CloseFile(HP_info* header_info)
{
    if (!header_info) {
        return 0;
    }
    int c = BF_CloseFile(header_info->fileDesc);
    if (c < 0) {
        return -1;
    }
    std::free(header_info->attrName);
    delete header_info;
    return 0;
}

union Records
{
    Record r;
    SecondaryRecordCustom s;
};

int insertEntry(HP_info header_info, Records record, bool secondaryRecord = false)
{
    int maxRecords = secondaryRecord ? SEC_MAX_RECORDS : MAX_RECORDS;
    size_t recordSize = secondaryRecord ? sizeof(SecondaryRecordCustom) : sizeof(Record);

    void* hpBlock = nullptr;
    void* block = nullptr;
    int blkCnt = BF_GetBlockCounter(header_info.fileDesc) - 1;
    char* insertHere = nullptr; // insertHere refers to the address of the block's record that we inserted
                                // if its value is not NULL, it indicates that the record we search has been found
    int blockId = 0;
    int unpackedField = maxRecords + 1; // unpacked field has 15 spaces, plus one for the pointer to the next block

    //
    // Insert record
    //
    int r = BF_ReadBlock(header_info.fileDesc, 0, &hpBlock);
    if (r < 0) {
        return -1;
    }
    std::unique_ptr<HeapFileInfo> heapFile(HeapFileInfo::createFromBlock(hpBlock));

    // allocate new block if this is the first entry to the file or if the last block is full
    if (heapFile->totalRecords == 0 || heapFile->totalRecords == maxRecords * blkCnt) {
        r = BF_AllocateBlock(header_info.fileDesc);
        if (r < 0) {
            return -1;
        }
        blkCnt++;
        r = BF_ReadBlock(header_info.fileDesc, blkCnt, &block);
        if (r < 0) {
            return -1;
        }

        //
        // Initialize block to invalid values
        //
        char* blockAddress = static_cast<char*>(block);
        char* packingAddress = blockAddress + recordSize * maxRecords;
        char* nextBlockAddress = packingAddress + sizeof(int) * maxRecords;
        std::memset(blockAddress, BLOCK_INVALID, recordSize * maxRecords);
        std::memset(packingAddress, PACKING_EMPTY, sizeof(int) * maxRecords);
        std::memset(nextBlockAddress, BLOCK_INVALID, sizeof(int));

        // insert record
        std::memcpy(blockAddress, &record, recordSize);
        std::memcpy(packingAddress, &PACKING_FILLED, sizeof(int));

        //
        // change pointer in previous block
        //
        if (blkCnt == 1) {
            heapFile->nextBlock = blkCnt;
        } else if (blkCnt > 1) {
            r = BF_ReadBlock(header_info.fileDesc, blkCnt - 1, &block);
            if (r < 0) {
                return -1;
            }
            char* address = static_cast<char*>(block) + maxRecords * recordSize + (unpackedField - 1) * sizeof(int);
            std::memcpy(address, &blkCnt, sizeof(int));             // pointer's new value is the number of current block
            // save changes of last two blocks in disk
            r = BF_WriteBlock(header_info.fileDesc, blkCnt - 1);    // previous
            if (r < 0) {
                return -1;
            }
        }
        r = BF_WriteBlock(header_info.fileDesc, blkCnt);        // current
        if (r < 0) {
            return -1;
        }
        blockId = blkCnt;
    } else {
        // find first empty space in blocks
        for (int j = 1; j <= blkCnt; j++) {   // for blocks
            r = BF_ReadBlock(header_info.fileDesc, j, &block);
            if (r < 0) {
                return -1;
            }
            char* address = static_cast<char*>(block);
            address += maxRecords * recordSize;
            int searching = 0;
            for (int i = 1; i <= maxRecords; i++) {   // for records of blocks
                // check the unpacked field and find which record in block is empty (0 -> empty, 1 -> written)
                std::memcpy(&searching, address, sizeof(int));
                if (searching == PACKING_EMPTY) {
                    insertHere = static_cast<char*>(block) + (i - 1) * recordSize;  // i refers to the number of the record
                    std::memcpy(insertHere, &record, recordSize);
                    std::memcpy(address, &PACKING_FILLED, sizeof(int));
                    break;
                }
                address += sizeof(int);
            }
            // save block changes to disk
            if (insertHere != nullptr) {
                r = BF_WriteBlock(header_info.fileDesc, j);
                if (r < 0) {
                    return -1;
                }
                blockId = j;    // the number of the block that record inserted
                break;
            }
        }
    }

    //
    // Update block_0
    //
    r = BF_ReadBlock(header_info.fileDesc, 0, &hpBlock);
    if (r < 0) {
        return -1;
    }
    heapFile->totalRecords++;
    heapFile->saveToBlock(hpBlock);
    r = BF_WriteBlock(header_info.fileDesc, 0);
    if (r < 0) {
        return -1;
    }
    return blockId;
}

int HP_InsertEntry(HP_info header_info, Record record)
{
    void* block = nullptr;
    int blkCnt = BF_GetBlockCounter(header_info.fileDesc) - 1;

    int currentId = 0;

    //
    // Check for duplicates
    //
    for (int j = 1; j <= blkCnt; j++) {   // for blocks
        int r = BF_ReadBlock(header_info.fileDesc, j, &block);
        if (r < 0) {
            return -1;
        }
        char* unpackedAddress = static_cast<char*>(block);
        unpackedAddress += MAX_RECORDS * sizeof(Record);
        int searchingFilled = 0;
        for (int i = 1; i <= MAX_RECORDS; i++) {  // for records of blocks
            // check the unpacked field and find which record in block is filled
            std::memcpy(&searchingFilled, unpackedAddress, sizeof(int));
            if (searchingFilled == PACKING_FILLED) {
                char* recordAddress = static_cast<char*>(block) + (i - 1) * sizeof(Record);
                std::memcpy(&currentId, recordAddress, sizeof(int));
                if (currentId == record.id) {
                    return -1;
                }
            }
            unpackedAddress += sizeof(int);
        }
    }

    Records r;
    r.r = record;
    return insertEntry(header_info, r);
}

int HP_SecondaryInsertEntry(HP_info header_info, SecondaryRecordCustom record)
{
    Records r;
    r.s = record;
    return insertEntry(header_info, r, true);
}

bool hasValidArguments(HP_info headerInfo, void* value)
{
    void* block = nullptr;
    // check if the key name and the key value are NULL
    if ((headerInfo.attrName == nullptr) && (value == nullptr)) {
        return false;
    }

    // check the number of total records in file from special-info
    // of block_0 and if it doesn't have any records return -1
    int r = BF_ReadBlock(headerInfo.fileDesc, 0, &block);
    if (r < 0) {
        return false;
    }
    std::unique_ptr<HeapFileInfo> heapFile(HeapFileInfo::createFromBlock(block));
    if (heapFile->totalRecords == 0) {
        return false;
    }
    return true;
}

Record recordFromAddress(char* address)
{
    Record rec;
    std::memcpy(&rec.id, address, sizeof(int));
    address += sizeof(rec.id);
    std::memcpy(&rec.name, address, sizeof(rec.name));
    address += sizeof(rec.name);
    std::memcpy(&rec.surname, address, sizeof(rec.surname));
    address += sizeof(rec.surname);
    std::memcpy(&rec.address, address, sizeof(rec.address));
    address += sizeof(rec.address);
    return rec;
}

SecondaryRecordCustom secondaryRecordFromAddress(char* address)
{
    SecondaryRecordCustom rec;
    std::memcpy(&rec.surname, address, sizeof(rec.surname));
    address += sizeof(rec.surname);
    std::memcpy(&rec.blockId, address, sizeof(int));
    address += sizeof(rec.blockId);
    return rec;
}

bool compareRecords(HP_info headerInfo, Record rec, void* value)
{
    return !std::strcmp(headerInfo.attrName, "id") && (rec.id == *static_cast<int*>(value));
}

int HP_DeleteEntry(HP_info header_info, void* value)
{
    void* block = nullptr;
    char* deleteHere = nullptr; // deleteHere refers to the address of the block's record that we deleted
                                // if its value is not NULL, it indicates that the record we search has been found
    int blkCnt = BF_GetBlockCounter(header_info.fileDesc) - 1;

    if (!hasValidArguments(header_info, value)) {
        return -1;
    }

    //
    // Delete
    //
    for (int j = 1; j <= blkCnt; j++) {                 // for blocks
        int r = BF_ReadBlock(header_info.fileDesc, j, &block);
        if (r < 0) {
            return -1;
        }
        for (int i=1; i <= MAX_RECORDS; i++) {           // for records of blocks
            char* address = static_cast<char*>(block);
            address += (i - 1) * sizeof(Record);        // address at the start of the record
            Record rec = recordFromAddress(address);    // get record info
            // check what's the name of the key and if current record's key equals to value
            if (compareRecords(header_info, rec, value)) {
                //
                // change id to -1 and fill rest fields of record with empty chars
                //
                deleteHere = static_cast<char*>(block) + (i - 1) * sizeof(Record);  // go to the start of the record to be deleted
                std::memcpy(deleteHere, &BLOCK_INVALID, sizeof(int));
                deleteHere += sizeof(int);
                char empty = '\0';
                std::memset(deleteHere, empty, sizeof(rec.name) + sizeof(rec.surname) + sizeof(rec.address));
                //
                // change record's unpacked field from 1 to 0
                // unpacked field is located after the end of fifth record
                //
                deleteHere = static_cast<char*>(block) + MAX_RECORDS * sizeof(Record) + (i - 1) * sizeof(int);   // i refers to the number of the record
                std::memcpy(deleteHere, &PACKING_EMPTY, sizeof(int));
                break;
            }
        }

        if (deleteHere != nullptr) {
            //
            // save changes of block that record deleted in disk
            //
            int r = BF_WriteBlock(header_info.fileDesc, j); // j is the number of current block
            if (r < 0) {
                return -1;
            }
            break;
        } else if (j == blkCnt) {
            //
            // reached last block and didn't find it
            //
            return -1;
        }
    }

    return 0;
}

int HP_GetAllEntries(HP_info header_info, void* value)
{
    void* block = nullptr;
    int blkCnt = BF_GetBlockCounter(header_info.fileDesc) - 1;
    int found = 0;
    int blocksRead = 0;

    if (!hasValidArguments(header_info, value)) {
        return -1;
    }

    //
    // Search
    //
    for (int j=1; j <= blkCnt; j++) {   // for blocks
        int r = BF_ReadBlock(header_info.fileDesc, j, &block);
        if (r < 0) {
            return -1;
        }
        for (int i=1; i <= MAX_RECORDS; i++) {    // for records of blocks
            char* address = static_cast<char*>(block);
            address += (i - 1) * sizeof(Record);
            Record rec = recordFromAddress(address);    // get record info
            // check what's the name of the key and if current record's key equals to value
            if (compareRecords(header_info, rec, value)) {
                found = 1;
                blocksRead = j; // number of blocks that have been read equals to the number of the current block
                std::cout << "ID\t\tNAME\t\tSURNAME\t\tADDRESS" << std::endl;
                std::cout << rec.id << "\t\t" << rec.name << "\t\t" << rec.surname << "\t\t" << rec.address << std::endl;
            }
        }
        //
        // reached last block and didn't find it
        //
        if (j == blkCnt && !found) {
            std::cout << "Didn't find any record with this key" << std::endl;
            return -1;
        }
    }

    return blocksRead;
}

int HP_GetAllEntriesAtBlock(HP_info header_info, void *value, int blockId)
{
    void* block = nullptr;
    int r = BF_ReadBlock(header_info.fileDesc, blockId + 1, &block);
    if (r < 0) {
        return -1;
    }
    for (int i=1; i <= MAX_RECORDS; i++) {    // for records of blocks
        char* address = static_cast<char*>(block);
        address += (i - 1) * sizeof(Record);
        Record rec = recordFromAddress(address);    // get record info
        // check what's the name of the key and if current record's key equals to value
        if (std::strcmp(rec.surname, static_cast<char*>(value)) == 0) {
            std::cout << "ID\t\tNAME\t\tSURNAME\t\tADDRESS" << std::endl;
            std::cout << rec.id << "\t\t" << rec.name << "\t\t" << rec.surname << "\t\t" << rec.address << std::endl;
        }
    }
    return 1;
}

std::vector<std::pair<int, int>> HP_SecondaryGetBlocks(HP_info headerInfo, void* value)
{
    std::vector<std::pair<int, int>> blocks = {};
    void* block = nullptr;
    int blkCnt = BF_GetBlockCounter(headerInfo.fileDesc) - 1;
    int blocksRead = 0;

    if (!hasValidArguments(headerInfo, value)) {
        return std::vector<std::pair<int, int>> { std::pair<int, int>{-1, -1} };
    }

    //
    // Search
    //
    for (int j=1; j <= blkCnt; j++) {   // for blocks
        int r = BF_ReadBlock(headerInfo.fileDesc, j, &block);
        if (r < 0) {
            return std::vector<std::pair<int, int>> { std::pair<int, int>(-1, -1) };
        }
        for (int i=1; i <= SEC_MAX_RECORDS; i++) {    // for records of blocks
            char* address = static_cast<char*>(block) + ((i - 1) * sizeof(SecondaryRecordCustom));
            SecondaryRecordCustom rec;
            std::memcpy(&rec, address, sizeof(SecondaryRecordCustom));
            // check what's the name of the key and if current record's key equals to value
            if (!std::strcmp(headerInfo.attrName, "surname") && !std::strcmp(rec.surname, static_cast<char*>(value))) {
                auto search = [&](std::pair<int, int> p) { return p.first == rec.blockId; };
                auto iter = std::find_if(blocks.begin(), blocks.end(), search);

                if (iter == blocks.end()) {
                    int temp = rec.blockId;
                    blocks.push_back(std::pair<int, int>(temp, 1));
                } else {
                    iter->second++;
                }
            }
        }
        blocksRead = j;
    }

    blocks.push_back(std::pair<int, int>(-1, blocksRead)); // last pair holds the number of blocks read

    return blocks;
}
