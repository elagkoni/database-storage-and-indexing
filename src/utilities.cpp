#include <iostream>
#include <cstring>
#include "record.h"
#include "utilities.h"

using namespace std;

/// strdup that uses the `new` operator
char* utilities::strdup(const char* target)
{
    char* returnString = new char[std::strlen(target) + 1];
    std::memcpy(returnString, target, std::strlen(target) + 1);
    return returnString;
}

string utilities::createFilePathString()
{
    string path = "";
#ifdef MAKEFILE // used when compiling with the provided makefile
    path = "./";
#else // used by QtCreator and Github Actions
    path = __FILE__;
    int index = path.find("freedom/");
    index += 8;
    path.erase(index);
#endif
    return path;
}

FILE* utilities::openRecordsFile(string fileName)
{
    string path = createFilePathString() + fileName;
    FILE* from = fopen(path.c_str(), "r");
    if (!from) {
        std::cout << "Error: Couldn't open records file" << std::endl;
    }
    return from;
}

void utilities::readRecordsFromFile(FILE* from, Record* records, int count)
{
    char buf[10];

    for (int i = 0; i < count; i++) {
        std::fscanf(from," {%[^,]", buf);
        records[i].id = atoi(buf);
        std::fscanf(from, "%[^\"]", buf);
        std::fscanf(from, " \"%[^\"]\" ", records[i].name);
        std::fscanf(from, "%[^\"]", buf);
        std::fscanf(from, " \"%[^\"]\" ", records[i].surname);
        std::fscanf(from, "%[^\"]", buf);
        std::fscanf(from, " \"%[^\"]\" ", records[i].address);
        std::fscanf(from, "%[^{]", buf);
    }
}
