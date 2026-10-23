#ifndef RECORD_H
#define RECORD_H

typedef struct {
    int id;
    char name[15];
    char surname[25];
    char address[50];
} Record;

typedef struct {
    Record record;
    int blockId;
} SecondaryRecord;

typedef struct {
    char surname[25];
    int blockId;
} SecondaryRecordCustom;

#endif // RECORD_H
