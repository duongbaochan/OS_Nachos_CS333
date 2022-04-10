#include "syscall.h"

int main()
{
    int size;
    char result[255];
    int fInId, fOutId;
    char *buffer = "abcdefghighk";

    // Mo file de ghi buffer vao
    fInId = Open("dbchan.txt");
    Write(buffer, 5, fInId);
    Close(fInId);
    // Mo file de doc
    fOutId = Open("dbchan.txt");

    // Doc toi da 255 ky tu
    size = Read(result, 255, fOutId);

    PrintString("\nSo ky tu doc duoc: ");
    PrintNum(size);
    PrintChar('\n');
    PrintString(result);
    Close(fOutId);

    Halt();
}