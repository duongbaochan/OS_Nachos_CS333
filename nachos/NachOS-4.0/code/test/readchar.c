#include "syscall.h"

int main()
{
    char read_char;
    read_char = ReadChar();
    Halt();
}