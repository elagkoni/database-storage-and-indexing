#ifndef BLOCK_H
#define BLOCK_H

#include "record.h"
#define BLOCK_SIZE 512

const int BLOCK_INVALID = -1;
const int PACKING_EMPTY = 0;
const int PACKING_FILLED = 1;
const int MAX_RECORDS = (BLOCK_SIZE - sizeof(int)) / (sizeof(Record) + sizeof(int)); // One pointer to the next block + k*(record + validity field)
const int SEC_MAX_RECORDS = (BLOCK_SIZE - sizeof(int)) / (sizeof(SecondaryRecordCustom) + sizeof(int));

#endif // BLOCK_H
