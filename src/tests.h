#ifndef TESTS_H
#define TESTS_H

#include "record.h"

extern const int RECORDS_COUNT;
extern const int RECORDS_DUPLICATES_COUNT;
extern char* HEAPFILE_NAME;
extern char* HASHTABLE_NAME;
extern char* SECONDARY_NAME;
extern char* INVALID_FILE_NAME;

void testHeapfile(Record* records);
void testHeapfileDuplicates(Record* records);
void testHashtableNormal(Record* records);
void testHashtableOptimalBuckets(Record* records);
void testHashtableWorstCase(Record* records);
void testSecondaryNormal(Record* records);

#endif // TESTS_H
