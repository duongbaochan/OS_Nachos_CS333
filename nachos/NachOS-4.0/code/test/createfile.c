#include "syscall.h"
//#include "copyright.h"
#define maxlen 32

int main()
{
    int check;
    check = Create("test.txt");

    Halt();
    return 0;
}
