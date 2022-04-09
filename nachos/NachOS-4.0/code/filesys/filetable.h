// Data structure to manage files in one process in Nachos
#ifndef FILE_TABLE_H
#define FILE_TABLE_H

#include "openfile.h"

#define MAX_FILE 10
#define MAX_STRING 50

struct FileLoad
{
    char *name;
};

class FileTable
{
private:
    FileLoad fileTable[MAX_FILE];
    int curOffset[MAX_FILE];

public:
    FileTable()
    {
        for (int i = 0; i < MAX_FILE; ++i)
        {
            curOffset[i] = 1;
        }
    }
    int Open(char *name)
    {
        for (int i = 0; i < MAX_FILE; i++)
        {
            // File Slot Available (curOffset = 1)
            // File Slot Used (curOffset = 0)
            if (curOffset[i] == 1)
            {
                fileTable[i].name = new char[MAX_FILE];
                fileTable[i].name = name;
                curOffset[i] = 0;
                return i;
            }
        }

        // Table full
        return -2;
    }
    int Close(int fid)
    {
        for (int i = 0; i < MAX_FILE; i++)
        {
            if (i = fid)
            {
                delete fileTable[i].name;
            }
        }
    }

    bool Contains(char *name)
    {
        for (int i = 0; i < MAX_FILE; i++)
            if (fileTable[i].name = name)
                return true;

        return false;
    }
};
#endif