#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>

#include "hashtable.h"
#include "hashtableinfo.h"
#include "heapfile.h"
#include "heapfileinfo.h"
#include "secondary.h"
#include "files.h"
#include "utilities.h"

int HashStatistics(char* fileName)
{
    int bucketCount = -1;
    std::string originalName = "";

    // open file and get required information
    HT_info* hinfo = HT_OpenIndex(fileName);
    if (hinfo) {
        bucketCount = hinfo->bucketCount;
        originalName = hinfo->originalName;
        HT_CloseIndex(hinfo);
    } else {
        SHT_info* shinfo = SHT_OpenSecondaryIndex(fileName);
        if (!shinfo) {
            return -1;
        }
        bucketCount = shinfo->bucketCount;
        originalName = shinfo->originalName;
        SHT_CloseSecondaryIndex(shinfo);
    }
    std::cout << "Hash Statistics for: " << fileName << std::endl;

    int totalBlocks = 1;            // blocks with records + info blocks
    int totalRecords = 0;
    int maxRecords = -1;            // maximum number of records in a block
    int minRecords = -1;            // minimum number of records in a block
    int bucketsWithOverflows = 0;   // number of buckets with overflow blocks

    // go through every heapfile and collect the required information
    for (int i = 0; i < bucketCount; i++) {
        std::string s(originalName + std::to_string(i));
        HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));
        if (!hp) { // empty bucket
            minRecords = 0;
            totalBlocks++;
            continue;
        }
        HeapFileInfo hpInfo(*hp);
        totalBlocks += hpInfo.blockCount();
        totalRecords += hp->totalRecords;
        int min = hp->totalRecords;
        if (min < minRecords || minRecords == -1) {
           minRecords = min;
        }
        int max = hp->totalRecords;
        if (max > maxRecords || maxRecords == -1) {
           maxRecords = max;
        }
        if (hpInfo.blockCount() > 2) {
            bucketsWithOverflows++;
        }
        HP_CloseFile(hp);
    }

    //
    // Printing
    //
#ifndef TESTING
    std::ostream outputStream(std::cout.rdbuf());
#else
    std::ofstream outputStream("./statistics.test");
#endif
    // a
    outputStream << "Total blocks used (including Hashtable and Heapfile info blocks): " << totalBlocks << std::endl;
    outputStream << "Block count (total blocks - info blocks): " << totalBlocks - bucketCount - 1 << std::endl;
    // b
    outputStream << "Minimum records bucket: " << minRecords << std::endl;
    outputStream << "Records per bucket (average): " << double(totalRecords) / bucketCount << std::endl;
    outputStream << "Maximum records bucket: " << maxRecords << std::endl;
    // c
    outputStream << "Blocks per bucket (average): " << (totalBlocks - bucketCount - 1) / bucketCount << std::endl;
    // d
    outputStream << "Number of buckets with overflow blocks: " << bucketsWithOverflows << std::endl;
    for (int i = 0; i < bucketCount; i++) {
        std::string s(originalName + std::to_string(i));
        HP_info* hp = HP_OpenFile(const_cast<char*>(s.c_str()));
        if (!hp) { // empty bucket
            HP_CloseFile(hp);
            continue;
        }
        HeapFileInfo hpInfo(*hp);
        int blocks = hpInfo.blockCount();
        if (blocks > 2) {
            outputStream << "Bucket " << i << ", number of overflow blocks: " << blocks - 2 << std::endl;
        }
        HP_CloseFile(hp);
    }

    return 0;
}
