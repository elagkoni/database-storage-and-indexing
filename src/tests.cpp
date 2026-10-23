#include <iostream>
#include <fstream>
#include <assert.h>
#include <unistd.h>

#include "tests.h"
#include "hashtable.h"
#include "secondary.h"
#include "hashstatistics.h"
#include "heapfile.h"
#include "utilities.h"
#include "record.h"

using namespace std;
using namespace utilities;

const int RECORDS_COUNT = 109;
const int RECORDS_DUPLICATES_COUNT = 5;
char* HEAPFILE_NAME = (char*)"heapfile_test";
char* HASHTABLE_NAME = (char*)"hashtable_test";
char* SECONDARY_NAME = (char*)"secondary_test";
char* INVALID_FILE_NAME = (char*)"invalid_file";

//---------------------------------------------------------
//   heapfile tests
//---------------------------------------------------------

void testHeapfile(Record* records)
{
    // HP_CreateFile
    assert(HP_CreateFile(HEAPFILE_NAME, 'i', (char*)"id", 2) == 0);
    // HP_OpenFile
    HP_info* hpinfo = HP_OpenFile(INVALID_FILE_NAME);
    assert(!hpinfo);
    hpinfo = HP_OpenFile(HEAPFILE_NAME);
    assert(hpinfo);
    // HP_InsertEntry
    for (int i = 0; i < RECORDS_COUNT; i++) {
        assert(HP_InsertEntry(*hpinfo, records[i]) > 0);
    }
    // HP_GetAllEntries
    int id = 2;
    assert(HP_GetAllEntries(*hpinfo, &id) > 0);
    // HP_DeleteEntry
    id = 1;
    assert(HP_GetAllEntries(*hpinfo, &id) == 1);
    assert(HP_DeleteEntry(*hpinfo, &id) == 0);
    assert(HP_GetAllEntries(*hpinfo, &id) == -1);
    assert(HP_InsertEntry(*hpinfo, records[id]) > 0);
    // HP_CloseFile
    assert(HP_CloseFile(hpinfo) == 0);
    // Remove all created files
    remove(HEAPFILE_NAME);
}

void testHeapfileDuplicates(Record* records)
{
    // HP_CreateFile
    assert(HP_CreateFile(HEAPFILE_NAME, 'i', (char*)"id", 2) == 0);
    // HP_OpenFile
    HP_info* hpinfo = HP_OpenFile(HEAPFILE_NAME);
    assert(hpinfo);
    // HP_InsertEntry
    assert(HP_InsertEntry(*hpinfo, records[0]) > 0);
    for (int i = 1; i < RECORDS_DUPLICATES_COUNT; i++) {
        assert(HP_InsertEntry(*hpinfo, records[i]) == -1);
    }
    // HP_CloseFile
    assert(HP_CloseFile(hpinfo) == 0);
    // Remove all created files
    remove(HEAPFILE_NAME);
}

//---------------------------------------------------------
//   hashtable tests
//---------------------------------------------------------

#define HASHTABLE_SETUP_TEST(j, b)                                  \
    assert(HT_CreateIndex(HASHTABLE_NAME, 'i', (char*)"id", 2, b) == 0);   \
    HT_info* htinfo = HT_OpenIndex(INVALID_FILE_NAME);              \
    assert(!htinfo);                                                \
    htinfo = HT_OpenIndex(HASHTABLE_NAME);                          \
    assert(htinfo);                                                 \
    for (int i = 0; i < RECORDS_COUNT; i += j) {                    \
        assert(HT_InsertEntry(*htinfo, records[i]) > 0);            \
    }

#define HASHTABLE_CLEAN_TEST                                        \
    remove(HASHTABLE_NAME);                                         \
    for (int i = 0; i < htinfo->bucketCount; i++) {                 \
        remove((std::string(HASHTABLE_NAME) + to_string(i)).c_str()); \
    }                                                               \
    assert(HT_CloseIndex(htinfo) == 0);                             \

void testHashStatistics(const string ref)
{
    HashStatistics(HASHTABLE_NAME);
    string path = createFilePathString() + "test_refs/" + ref;
    ifstream testFile("./statistics.test");
    ifstream refFile(path);
    string testLine;
    string refLine;
    while (getline(testFile, testLine)) {
        getline(refFile, refLine);
        if (!testLine.empty() && testLine.back() == '\r') testLine.pop_back();
        if (!refLine.empty() && refLine.back() == '\r') refLine.pop_back();
        assert(testLine == refLine);
    }
}

// Typical case
void testHashtableNormal(Record* records)
{
    // HT_CreateIndex && HT_OpenIndex && HT_InsertEntry
    HASHTABLE_SETUP_TEST(1, 5) // 1 means insert all records, 5 means 5 buckets
    // HT_GetAllEntries
    int id = 3;
    assert(HT_GetAllEntries(*htinfo, &id) > 0);
    // HashStatistics
    testHashStatistics("statistics0.ref");
    // HT_DeleteEntry
    id = 2;
    assert(HT_GetAllEntries(*htinfo, &id) == 1);
    assert(HT_DeleteEntry(*htinfo, &id) == 0);
    assert(HT_GetAllEntries(*htinfo, &id) == -1);
    assert(HT_InsertEntry(*htinfo, records[id]) > 0);
    // HT_CloseIndex
    HASHTABLE_CLEAN_TEST
}

// Just enough buckets to not use overflow blocks
void testHashtableOptimalBuckets(Record* records)
{
    // HT_CreateIndex && HT_OpenIndex && HT_InsertEntry
    HASHTABLE_SETUP_TEST(1, 25) // 1 means insert all records, 25 means 25 buckets
    // HT_GetAllEntries
    int id = 3;
    assert(HT_GetAllEntries(*htinfo, &id) > 0);
    // HashStatistics
    testHashStatistics("statistics2.ref");
    // HT_DeleteEntry
    id = 2;
    assert(HT_GetAllEntries(*htinfo, &id) == 1);
    assert(HT_DeleteEntry(*htinfo, &id) == 0);
    assert(HT_GetAllEntries(*htinfo, &id) == -1);
    assert(HT_InsertEntry(*htinfo, records[id]) > 0);
    // HT_CloseIndex
    HASHTABLE_CLEAN_TEST
}

// Only one bucket is used (worst case for hashing)
void testHashtableWorstCase(Record* records)
{
    // HT_CreateIndex && HT_OpenIndex && HT_InsertEntry
    HASHTABLE_SETUP_TEST(5, 5) // 5 means 1 out of every 5 records
    // HT_GetAllEntries
    int id = 3;
    assert(HT_GetAllEntries(*htinfo, &id) == -1);
    // HashStatistics
    testHashStatistics("statistics1.ref");
    // HT_DeleteEntry
    id = 0;
    assert(HT_GetAllEntries(*htinfo, &id) == 1);
    assert(HT_DeleteEntry(*htinfo, &id) == 0);
    assert(HT_GetAllEntries(*htinfo, &id) == -1);
    int missingId = 1;
    assert(HT_DeleteEntry(*htinfo, &missingId) == -1);
    // HT_CloseIndex
    HASHTABLE_CLEAN_TEST
}

//---------------------------------------------------------
//   secondary hashtable tests
//---------------------------------------------------------

#define SECONDARY_SETUP_TEST(j, b)                                      \
    assert(HT_CreateIndex(HASHTABLE_NAME, 'i', (char*)"id", 2, b) == 0);    \
    assert(SHT_CreateSecondaryIndex(SECONDARY_NAME, (char*)"surname", 7, b, HASHTABLE_NAME) == 0);  \
    HT_info* htinfo = HT_OpenIndex(HASHTABLE_NAME);                     \
    assert(htinfo);                                                     \
    SHT_info* shtinfo = SHT_OpenSecondaryIndex(INVALID_FILE_NAME);      \
    assert(!shtinfo);                                                   \
    shtinfo = SHT_OpenSecondaryIndex(SECONDARY_NAME);                   \
    assert(shtinfo);                                                    \
    SecondaryRecord srec;                                               \
    for (int i = 0; i < RECORDS_COUNT; i += j) {                        \
        srec.record = records[i];                                       \
        srec.blockId = HT_InsertEntry(*htinfo, records[i]);             \
        assert(SHT_SecondaryInsertEntry(*shtinfo, srec) == 0);          \
    }

#define SECONDARY_CLEAN_TEST                                            \
    remove(HASHTABLE_NAME);                                             \
    for (int i = 0; i < htinfo->bucketCount; i++) {                     \
        remove((std::string(HASHTABLE_NAME) + to_string(i)).c_str());   \
    }                                                                   \
    assert(HT_CloseIndex(htinfo) == 0);                                 \
    remove(SECONDARY_NAME);                                             \
    for (int i = 0; i < shtinfo->bucketCount; i++) {                    \
        remove((std::string(SECONDARY_NAME) + to_string(i)).c_str());   \
    }                                                                   \
    assert(SHT_CloseSecondaryIndex(shtinfo) == 0);                      \

void testSecondaryNormal(Record* records)
{
    // SHT_CreateIndex && SHT_OpenSecondaryIndex && SHT_InsertSecondaryEntry
    SECONDARY_SETUP_TEST(1, 5) // 1 means insert all records, 5 means 5 buckets
    // SHT_SecondaryGetAllEntries
    char* surname = (char*)"surname_17";
    assert(SHT_SecondaryGetAllEntries(*shtinfo, *htinfo, (void*)surname) > 0);
    surname = (char*)"surname";
    assert(SHT_SecondaryGetAllEntries(*shtinfo, *htinfo, (void*)surname) == -1);
    // SHT_CloseSecondaryIndex
    SECONDARY_CLEAN_TEST
}
