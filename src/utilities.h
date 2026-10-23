#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include "record.h"

namespace utilities {
    char* strdup(const char* target);
    std::string createFilePathString();
    FILE* openRecordsFile(std::string fileName);
    void readRecordsFromFile(FILE* from, Record* records, int count);
};

#endif // UTILITIES_H
