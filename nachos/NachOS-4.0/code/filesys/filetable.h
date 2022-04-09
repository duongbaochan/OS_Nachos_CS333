// Data structure to manage files in one process in Nachos
#ifndef FILE_TABLE_H
#define FILE_TABLE_H

#include "openfile.h"

#define MAX_FILE 10

class FileTable
{
private:
    OpenFile *fileTable[MAX_FILE];
    int curOffset[MAX_FILE];

public:
    FileTable()
    {
        for (int i = 0; i < MAX_FILE; ++i)
        {
            fileTable[i] = NULL;
            curOffset[i] = 1;
        }
    }
    int Open(OpenFile *file)
    {
        for (int i = 0; i < MAX_FILE; i++)
        {
            // If file is already in the table;
            if (fileTable[i] == file)
                return -1;

            // File Slot Available (curOffset = 1)
            // File Slot Used (curOffset = 0)
            if (curOffset[i] == 1)
            {
                fileTable[i] = file;
                curOffset[i] = 0;
                break;
            }
        }
    }
    int Close(int fid);
};
#endif