#include "syscall.h"

int main()
{
    // Group information
    PrintString("Group information: \n");
    PrintString("1/ Nguyen Duc Thang (19125121)\n");
    PrintString("2/ Dinh Duy Phuoc (19125115)\n");
    PrintString("3/ Duong Bao Chan (19125080)\n");

    // ASCII program
    PrintString("ASCII program: \n");
    PrintString("Print all readable characters on ascii table (32-126).\n");

    // Sort program
    PrintString("Sort program: \n");
    PrintString("Ask user to input the amount of numbers.\n");
    PrintString("Then, ask user to choose between sorting increase or decrease (0 for increase, 1 for decrease).\n");
    PrintString("Print sorted array using bubble sort.\n");

    Halt();
}