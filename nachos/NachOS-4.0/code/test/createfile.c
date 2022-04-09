#include "syscall.h"
#define maxlen 32

int main()
{
    int checkCreate, checkOpen1, checkOpen2;

    checkCreate = Create("world.txt");
    checkOpen1 = Open("hello.txt");

    // // checkOpen2 = Open("hello.txt");

    // PrintNum(checkOpen1);
    // // PrintNum(checkOpen2);

    Halt();

    return 0;
}
