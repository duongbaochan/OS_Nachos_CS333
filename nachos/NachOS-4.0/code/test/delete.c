#include "syscall.h"

int main()
{
    int check;
    check = Remove("test.txt");

    Halt();
    return 0;
}
