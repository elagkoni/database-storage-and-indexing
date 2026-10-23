#include <iostream>
#include <fstream>
#include <assert.h>
#include <unistd.h>

extern "C" {
      #include "../lib/BF.h"
}

#include "hashtable.h"
#include "secondary.h"
#include "hashstatistics.h"
#include "heapfile.h"
#include "utilities.h"
#include "tests.h"

using namespace std;
using namespace utilities;

//---------------------------------------------------------
//   main function
//---------------------------------------------------------

int main()
{
    BF_Init();

    FILE* from = openRecordsFile("records15K.txt");

#ifdef TESTING
    // read records from file
    Record records[109];
    readRecordsFromFile(from, records, 109);
    // special duplicate records array
    Record recordsDuplicates[] =
    {
        { 0,"name_0","surname_0","address_0" },
        { 0,"name_1","surname_1","address_1" },
        { 0,"name_2","surname_2","address_2" },
        { 0,"name_3","surname_3","address_3" },
        { 0,"name_4","surname_4","address_4" }
    };
    // run tests
    testHeapfile(records);
    testHeapfileDuplicates(recordsDuplicates);
    testHashtableNormal(records);
    testHashtableOptimalBuckets(records);
    testHashtableWorstCase(records);
    testSecondaryNormal(records);
#else
    // read records from file
    Record records[15000];
    readRecordsFromFile(from, records, 15000);
    // create hashtables and insert records
    int r = HT_CreateIndex(HASHTABLE_NAME, 'i', (char*)"id", 2, 3000);
    r = SHT_CreateSecondaryIndex(SECONDARY_NAME, (char*)"surname", 7, 3000, HASHTABLE_NAME);
    (void)r;
    HT_info* htinfo = HT_OpenIndex(HASHTABLE_NAME);
    SHT_info* shtinfo = SHT_OpenSecondaryIndex(SECONDARY_NAME);
    SecondaryRecord srec;
    for (int i = 0; i < 15000; i++) {
        srec.record = records[i];
        srec.blockId = HT_InsertEntry(*htinfo, records[i]);
        SHT_SecondaryInsertEntry(*shtinfo, srec);
    }
    // test random record
    int id = 7777;
    HT_GetAllEntries(*htinfo, &id);
    char* surname = (char*)"surname_0";
    SHT_SecondaryGetAllEntries(*shtinfo, *htinfo, surname);
    // hash statistics
    HashStatistics(HASHTABLE_NAME);
    HashStatistics(SECONDARY_NAME);
    // clean files and hashtables
    remove(HASHTABLE_NAME);
    for (int i = 0; i < htinfo->bucketCount; i++) {
        remove((std::string(HASHTABLE_NAME) + to_string(i)).c_str());
    }
    HT_CloseIndex(htinfo);
    remove(SECONDARY_NAME);
    for (int i = 0; i < shtinfo->bucketCount; i++) {
        remove((std::string(SECONDARY_NAME) + to_string(i)).c_str());
    }
    SHT_CloseSecondaryIndex(shtinfo);
#endif

    fclose(from);

    return 0;
}
