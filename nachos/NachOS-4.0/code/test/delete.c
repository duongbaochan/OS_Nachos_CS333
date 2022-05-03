#include "syscall.h"

int main()
{
    int check;
    check = Remove("ddphuoc123.txt");
    PrintNum(check);

    Halt();
    return 0;
}
