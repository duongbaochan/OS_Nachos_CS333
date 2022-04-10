#include "syscall.h"

int main()
{
    char result[255];
    int fOutId, fInId;
    int size;
    fOutId = Open("dbchan.txt");

    // Doc toi da 255 ky tu
    size = Read(result, 255, fOutId);
    Close(fOutId);

    fInId = Open("dbchan1.txt");
    Write(result, size, fInId);
    Close(fInId);

    Halt();

    return 0;
}