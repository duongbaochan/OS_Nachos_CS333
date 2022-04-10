#include "syscall.h"

int main()
{
    int c;
    int i;
    int j;

    char buffer[500];
    int fileid, filesz;

    fileid = Open("dbchan.txt");
    if (fileid == -1)
    {
        PrintString("Error reading file.");
        Halt();
    }

    filesz = Seek(-1, fileid);

    for (i = 0; i < filesz; i++)
    {
        Read(&c, 1, fileid);
        buffer[i] = c;
    }
    Close(fileid);

    fileid = Open("dbchan1.txt");
    if (fileid == -1)
    {
        PrintString("Error reading file.");
        Halt();
    }
    filesz = Seek(-1, fileid);

    for (j = 0; j < filesz; j++)
    {
        Read(&c, 1, fileid);
        buffer[i + j] = c;
    }
    Close(fileid);

    fileid = Open("dbchan123.txt");

    Write(buffer, i + j, fileid);
    Close(fileid);

    Halt();

    return 0;
}