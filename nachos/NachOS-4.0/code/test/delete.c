#include "syscall.h"

int main()
{
    int check;
    check = Remove("test.txt");
    PrintNum(check);

    Halt();
    return 0;
}
