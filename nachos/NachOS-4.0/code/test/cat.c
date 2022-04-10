#include "syscall.h"

int main()
{
    char result[255];
    int fOutId;
    int size;
    fOutId = Open("dbchan.txt");

    // Doc toi da 255 ky tu
    size = Read(result, 255, fOutId);

    PrintString("\nSo ky tu doc duoc: ");
    PrintNum(size);
    PrintChar('\n');
    PrintString(result);
    Close(fOutId);

    Halt();

    return 0;
}