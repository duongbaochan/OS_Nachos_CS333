#include "syscall.h"
//#include "copyright.h"
#define maxlen 32

int main()
{
    int check;
    check = Create("test.txt");
    check = Open("test.txt");
    Halt();
    return 0;
}
