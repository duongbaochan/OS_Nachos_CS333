#include "syscall.h"
#define maxlen 32

int main()
{
    // int checkCreate, checkOpen1, checkOpen2;

    // checkCreate = Create("world.txt");
    int t = Open("dbchan.txt");
    PrintNum(t);
    PrintNum(Close(t));
    PrintNum(Close(t));

    // // checkOpen2 = Open("hello.txt");

    // PrintNum(checkOpen1);
    // // PrintNum(checkOpen2);

    Halt();

    return 0;
}
