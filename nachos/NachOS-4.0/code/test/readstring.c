#include "syscall.h"
#define MAX_LENGTH 255

int main()
{
    char s[MAX_LENGTH];
    ReadString(s, MAX_LENGTH);
    // PrintString(s);
    Halt();
}