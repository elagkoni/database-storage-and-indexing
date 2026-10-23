#ifndef HEAPFILE_H
#define HEAPFILE_H
#include <vector>
#include "record.h"

#define HEAPFILE_MAKE_INVALID(x) ((x).fileDesc = -1)
#define HEAPFILE_IS_INVALID(x) ((x).fileDesc == -1)

typedef struct {
    int fileDesc;
    char attrType;
    char* attrName;
    int attrLength;
    int totalRecords;
    int nextBlock;
} HP_info;

int HP_CreateFile(char* fileName, char attrType, char* attrName, int attrLength);
HP_info* HP_OpenFile(char* fileName);
int HP_CloseFile(HP_info* header_info);
int HP_InsertEntry(HP_info header_info, Record record);
int HP_SecondaryInsertEntry(HP_info header_info, SecondaryRecordCustom record);
int HP_DeleteEntry(HP_info header_info, void* value);
int HP_GetAllEntries(HP_info header_info, void* value);
std::vector<std::pair<int, int> > HP_SecondaryGetBlocks(HP_info headerInfo, void* value);
int HP_GetAllEntriesAtBlock(HP_info header_info, void* value, int block);

#endif // HEAPFILE_H
