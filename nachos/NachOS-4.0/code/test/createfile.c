#include "syscall.h"
#define maxlen 32

int main()
{
    int checkCreate, checkOpen1, checkOpen2;

    checkCreate = Create("hello.txt");
    checkOpen1 = Open("hello");
    checkOpen2 = Open("hello");

    PrintNum(checkOpen1);
    PrintNum(checkOpen2);

    Halt();

    return 0;
}
