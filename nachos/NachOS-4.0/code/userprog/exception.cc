// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include "syscall.h"
#include "kernel.h"
#include "filetable.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

#define MAX_LENGTH_STRING 255
#define INT_MIN -2147483647
#define INT_MAX 2147483647
// Define max file name length
#define MAX_NAME_LENGTH 32
#define RW_ERROR_FILE -1
#define RW_END_FILE -2

FileTable table = FileTable();

void increasePC()
{
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

// Input: 	- Userspace address (int)
// 			- Length of user input string (int)
// 			- Buffer (char*)
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from kernel space to user space
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

char *User2System(int addr, int convert_length = -1)
{
	int length = 0;
	bool stop = false;
	char *str;

	do
	{
		int oneChar;
		kernel->machine->ReadMem(addr + length, 1, &oneChar);
		length++;
		// if convert_length == -1, we use '\0' to terminate the process
		// otherwise, we use convert_length to terminate the process
		stop = ((oneChar == '\0' && convert_length == -1) ||
				length == convert_length);
	} while (!stop);

	str = new char[length];
	for (int i = 0; i < length; i++)
	{
		int oneChar;
		kernel->machine->ReadMem(addr + i, 1,
								 &oneChar); // copy characters to kernel space
		str[i] = (unsigned char)oneChar;
	}
	return str;
}

void getUserKernelFileName(char *name)
{
	int address;
	address = kernel->machine->ReadRegister(4);

	// name = new char[MAX_NAME_LENGTH + 1];

	for (int i = 0; i < MAX_NAME_LENGTH; ++i)
	{
		int ch = 0;
		kernel->machine->ReadMem(address + i, 1, &ch);
		if (ch == 0)
			break;
		name[i] = (char)ch;
	}
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
		DEBUG(dbgAddr, "\n No valid translation found.");
		printf("\n\n No valid translation found.");
		SysHalt();
		break;
	case ReadOnlyException:
		DEBUG(dbgAddr, "\n Write attempted to page marked 'read-only'.");
		printf("\n\n Write attempted to page marked 'read-only'.");
		SysHalt();
		break;
	case BusErrorException:
		DEBUG(dbgAddr, "\n Translation resulted in an invalid physical address.");
		printf("\n\n Translation resulted in an invalid physical address.");
		SysHalt();
		break;
	case AddressErrorException:
		DEBUG(dbgAddr, "\n Unaligned reference or one that was beyond the end of the address space.");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space.");
		SysHalt();
		break;
	case OverflowException:
		DEBUG(dbgAddr, "\n Integer overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		SysHalt();
		break;
	case IllegalInstrException:
		DEBUG(dbgAddr, "\n Unimplemented or reserved instr.");
		printf("\n\n Unimplemented or reserved instr.");
		SysHalt();
		break;
	case NumExceptionTypes:
		DEBUG(dbgAddr, "\n Not using number type.");
		printf("\n\n Not using number type.");
		SysHalt();
		break;
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();
			ASSERTNOTREACHED();
			break;
		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			increasePC();

			return;

			ASSERTNOTREACHED();

		case SC_ReadNum:
			int res;
			bool isNegative;
			char c;
			c = kernel->synchConsoleIn->GetChar();
			isNegative = false;
			res = 0;
			if (c == '-') // Check negative integer
				isNegative = true;
			else if (c >= '0' && c <= '9') // Check whether input character is in range [0,9]
				res = res * 10 + (c - '0');
			else if (c == '\n')
			{
				res = 0;
				DEBUG(dbgSys, "\nERROR!!User has not entered anything yet!!!\n"
								  << res);
				printf("\nERROR!!User has not entered anything yet!!!");
				kernel->machine->WriteRegister(2, int(res));
				increasePC();
				return;
			}
			else
			{
				res = 0;
				DEBUG(dbgSys, "\nERROR!!User entered invalid integer!!!\n"
								  << res);
				printf("ERROR!!User entered invalid integer!!!");
				kernel->machine->WriteRegister(2, int(res));
				increasePC();
				return;
			}

			while ((c = kernel->synchConsoleIn->GetChar()) != '\n')
			{
				if (c >= '0' && c <= '9')
					res = res * 10 + (c - '0');
				else
				{
					res = 0;
					DEBUG(dbgSys, "\nERROR!!User entered invalid integer!!!\n"
									  << res);
					printf("\nERROR!!User entered invalid integer!!!");
					kernel->machine->WriteRegister(2, int(res));
					increasePC();
					return;
				}
			}
			if (isNegative)
				res = -res;
			if (res > INT_MAX)
			{ // Check whether input integer is larger than 32-bit integer
				res = 0;
				DEBUG(dbgSys, "\nERROR!!Input integer is out of range!!\n"
								  << res);
				printf("\nERROR!!Input integer is out of range large!!!");
				kernel->machine->WriteRegister(2, int(res));
				increasePC();
				return;
			}
			else if (res < INT_MIN)
			{ // Check whether input integer is smaller than 32-bit integer
				res = 0;
				DEBUG(dbgSys, "\nERROR!!Input integer is out of range!!\n"
								  << res);
				printf("\nERROR!!Input integer is out of range small!!!");
				kernel->machine->WriteRegister(2, int(res));
				increasePC();
				return;
			}
			if (isNegative && !res)
			{
				res = 0;
				DEBUG(dbgSys, "\nERROR!!User entered invalid integer!!!\n"
								  << res);
				kernel->machine->WriteRegister(2, int(res));
				increasePC();
				return;
			}

			DEBUG(dbgSys, "\nValid integer to read is: " << res << "\n");
			kernel->machine->WriteRegister(2, int(res));
			increasePC();
			return;

		case SC_PrintNum:
			int MAX_LENGTH_INT;
			MAX_LENGTH_INT = 11; // max length of 32-bit integer include sign
			char *buffer;
			buffer = new char[MAX_LENGTH_INT + 1];
			int n;
			n = 0;
			int number;
			number = kernel->machine->ReadRegister(4);

			if (number == 0)
				kernel->synchConsoleOut->PutChar('0');

			if (number == INT_MIN)
			{
				kernel->synchConsoleOut->PutChar('-');
				for (int i = 0; i < 10; ++i)
					kernel->synchConsoleOut->PutChar("2147483648"[i]);
				increasePC();
				delete buffer;
				return;
			}
			if (number < 0)
			{
				kernel->synchConsoleOut->PutChar('-');
				number = -number;
			}

			while (number)
			{
				buffer[n++] = number % 10;
				number /= 10;
			}
			for (int i = n - 1; i >= 0; --i)
				kernel->synchConsoleOut->PutChar(buffer[i] + '0');

			delete buffer;
			increasePC();
			return;

		case SC_ReadChar:
			char ch;
			char resu;
			resu = kernel->synchConsoleIn->GetChar();

			if (resu == '\n') // User does not input
			{
				DEBUG(dbgSys, "\nERROR!!User has not inputed anything!!!");
				printf("\nERROR!!User has not inputed anything!!!");
				increasePC();
				return;
			}
			else
			{
				ch = kernel->synchConsoleIn->GetChar();
				if (ch != '\n')
				{
					DEBUG(dbgSys, "\nERROR!!User inputs more than 1 character!!!");
					printf("\nERROR!!User inputs more than 1 character!!!");
					increasePC();
					return;
				}
			}

			DEBUG(dbgSys, "\nValid character to read is: " << resu << "\n");
			kernel->machine->WriteRegister(2, resu);
			increasePC();
			return;

		case SC_PrintChar:
			char cha;
			cha = (char)kernel->machine->ReadRegister(4);
			kernel->synchConsoleOut->PutChar(cha);
			increasePC();
			return;

		case SC_RandomNum:
			int res1;
			RandomInit(time(NULL));
			res1 = RandomNumber();
			kernel->machine->WriteRegister(2, res1);
			increasePC();
			return;

		case SC_ReadString:
			int virAddr; // declare address from input register
			int length;
			int inputLength;
			char *strName;
			char c1;

			virAddr = kernel->machine->ReadRegister(4);
			length = kernel->machine->ReadRegister(5);
			strName =
				new char[length]; // buffer returned to userspace
			inputLength = 0;
			while ((c1 = kernel->synchConsoleIn->GetChar()) != '\n')
			{
				strName[inputLength] = c1;
				inputLength++;
			}
			strName[inputLength] = '\0';

			int numBytes;
			numBytes = System2User(virAddr, inputLength, strName); // copy from kernel to user
			if (inputLength > length)
			{
				printf("\nInput string is out of required range.\n");
				increasePC();
				return;
			}
			if (numBytes == 0)
			{
				printf("Empty string.\n");
			}
			else if (numBytes > MAX_LENGTH_STRING)
			{
				printf("\nString too larger!!!");
				increasePC();
				return;
			}

			increasePC();
			return;

		case SC_PrintString:
			int memPtr;
			memPtr = kernel->machine->ReadRegister(4); // read address of C-string
			char *buff;
			buff = User2System(memPtr);
			for (int i = 0; i < strlen(buff); i++)
				kernel->synchConsoleOut->PutChar(buff[i]);
			delete[] buff;
			increasePC();
			return;

		case SC_Create:
			// Load file name from user
			int address;
			address = kernel->machine->ReadRegister(4);
			char *name;
			name = User2System(address);
			// getUserKernelFileName(name);

			// Create new file
			bool check;
			check = kernel->fileSystem->Create(name);

			if (check)
			{
				kernel->machine->WriteRegister(2, 0);
				printf("File Created Successfully...\n");
			}
			else
			{
				kernel->machine->WriteRegister(2, -1);
				printf("Failed Creating File...\n");
			}

			delete name;

			increasePC();
			return;

		case SC_Remove:
			// Load file name from user

			address = kernel->machine->ReadRegister(4);
			name = User2System(address);

			// getUserKernelFileName(name);

			// Remove file;
			check = kernel->fileSystem->Remove(name);

			if (check)
			{
				kernel->machine->WriteRegister(2, 0);
				printf("File Deleted Successfully...\n");
			}
			else
			{
				kernel->machine->WriteRegister(2, -1);
				printf("Failed Deleting File...\n");
			}

			delete name;

			increasePC();
			return;

		case SC_Open:
			// input: char*name, int type
			// output: 0 neu la input,1 la output,id neu thanh cong,-1 neuu that bai
			// muc dich:mo file
			OpenFile *file;
			int virAddr1;
			char *fileName;
			int freeSlot;
			// doc lenh tu thanh ghi
			virAddr1 = kernel->machine->ReadRegister(4);
			type = kernel->machine->ReadRegister(5);

			fileName = User2System(virAddr1, MAX_NAME_LENGTH);

			freeSlot = kernel->fileSystem->FindFreeSlot(); // kiem freeslot
			if (freeSlot != -1)
			{
				file = kernel->fileSystem->Open(fileName);

				if (file != NULL)
				{
					kernel->fileSystem->fileTable[freeSlot] = file;
					printf("Open file successfully!!\n");
					kernel->machine->WriteRegister(2, freeSlot); // mo file thanh cong
				}
				else
				{
					printf("Open file failed!!\n");
					kernel->machine->WriteRegister(2, -1);
				}
			}
			else
			{
				printf("Table is full!!!\n");
				kernel->machine->WriteRegister(2, -1);
			}
			delete[] fileName;
			increasePC();
			return;

		case SC_Close:

			// input : int id
			//  output: 0 thanh cong,1 that bai
			// muc dich:dong file
			int file_Id;
			file_Id = kernel->machine->ReadRegister(4); // doc tu thanh ghi

			if (file_Id >= 2 && file_Id <= MAX_FILE_OPEN - 1)
			{
				if (kernel->fileSystem->fileTable[file_Id] != NULL)
				{

					delete kernel->fileSystem->fileTable[file_Id]; // xoa danh dau trong fileTable

					kernel->fileSystem->fileTable[file_Id] = NULL;
					printf("Close file succesfully!!!\n");
					kernel->machine->WriteRegister(2, 0);
				}
				else
				{
					printf("This file has not opened yet!!!\n");
					kernel->machine->WriteRegister(2, -1);
				}
			}
			else
			{
				printf("Can not close Console IO!!!\n");
				kernel->machine->WriteRegister(2, -1);
			}

			increasePC();
			return;

		case SC_Read:
			// Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
			// Output: -1: Loi, So byte read thuc su: Thanh cong, -2: Thanh cong
			// Cong dung: Doc file voi tham so la buffer, so ky tu cho phep va id cua
			// file

			int virAddr_read;	// chua dia chi chuoi buffer
			int charcount_read; // chua charcount la so ky tu duoc doc
			int id_read;		// id cua file
			int curPosition_read;
			int newPosition_read;
			char *buffer_read;
			int readBytes_read;

			DEBUG(dbgFile, "\n SC_Read call ...");
			DEBUG(dbgFile, "\n Reading virtual address of buffer");
			virAddr_read = kernel->machine->ReadRegister(4);
			DEBUG(dbgFile, "\n Reading charcount.");
			charcount_read = kernel->machine->ReadRegister(5);
			DEBUG(dbgFile, "\n Reading id.");
			id_read = kernel->machine->ReadRegister(6);

			// Kiem tra file id co hop le
			if (id_read < 0 || id_read >= MAX_FILE_OPEN)
			{
				printf("\nInvalid file id.");
				kernel->machine->WriteRegister(2, -1);
				increasePC();
				return;
			}

			// Kiem tra file co ton tai khong
			if (kernel->fileSystem->fileTable[id_read] == NULL)
			{
				printf("\nFile does not exist in file table");
				kernel->machine->WriteRegister(2, RW_ERROR_FILE);
				increasePC();
				return;
			}

			// Truong hop doc file stdout
			if (id_read == INDEX_STDOUT)
			{
				printf("\nCan't read file stdout.");
				kernel->machine->WriteRegister(2, RW_ERROR_FILE);
				increasePC();
				return;
			}

			// Truong hop file doc duoc

			curPosition_read = kernel->fileSystem->fileTable[id_read]
								   ->getCurrentOffset(); // Lay vi tri current position
			buffer_read = new char[charcount_read];

			// Truong hop doc file binh thuong
			if ((kernel->fileSystem->fileTable[id_read]->Read(buffer_read, charcount_read)) > 0)
			{
				// So byte thuc su = newPosition - curPosition
				newPosition_read = kernel->fileSystem->fileTable[id_read]->getCurrentOffset();
				// Copy chuoi tu vung nho System Space sang User Space voi buffer co do
				// dai la readBytes (so byte that su da doc)
				readBytes_read = newPosition_read - curPosition_read;
				System2User(virAddr_read, readBytes_read, buffer_read);

				kernel->machine->WriteRegister(2, readBytes_read);
			}
			else // Truong hop con lai: doc va cham toi cuoi file tra ve -2
			{
				kernel->machine->WriteRegister(2, RW_END_FILE);
			}

			delete buffer_read;
			increasePC();
			return;

		case SC_Write:
			// Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
			// Output: -1: Loi, So byte write thuc su: Thanh cong, -2: Thanh cong
			// Cong dung: Ghi file voi tham so la buffer, so ky tu cho phep va id cua
			// file
			int virAddr_write;	 // chua dia chi chuoi buffer
			int charcount_write; // chua charcount la so ky tu duoc doc
			int id_write;		 // id cua file

			int curPosition_write;
			int newPosition_write;
			char *buffer_write;
			int readBytes_write;

			DEBUG(dbgFile, "\n SC_Read call ...");
			DEBUG(dbgFile, "\n Reading virtual address of buffer");
			virAddr_write = kernel->machine->ReadRegister(4);
			DEBUG(dbgFile, "\n Reading charcount.");
			charcount_write = kernel->machine->ReadRegister(5);
			DEBUG(dbgFile, "\n Reading id.");
			id_write = kernel->machine->ReadRegister(6);

			// Kiem tra file id co hop le
			if (id_write < 0 || id_write > MAX_FILE_OPEN)
			{
				printf("\nInvalid file id.");
				kernel->machine->WriteRegister(2, RW_ERROR_FILE);
				increasePC();
				return;
			}

			// Kiem tra file co ton tai khong
			if (kernel->fileSystem->fileTable[id_write] == NULL)
			{
				printf("\nFile does not exist in file table");
				kernel->machine->WriteRegister(2, RW_ERROR_FILE);
				increasePC();
				return;
			}

			// Xet truong hop ghi vao file only read hoac file stdin thi tra ve
			// RW_ERROR_FILE
			if (id_write == INDEX_STDIN)
			{
				printf("\nCan't write in file stdin.");
				kernel->machine->WriteRegister(2, RW_ERROR_FILE);
				increasePC();
				return;
			}

			curPosition_write = kernel->fileSystem->fileTable[id_write]
									->getCurrentOffset(); // Kiem tra thanh cong thi lay vi
														  // tri curPosition
			buffer_write = User2System(virAddr_write,
									   charcount_write); // Copy vung nho User Space sang System
														 // space voi buffer dai charcount bytes

			if (id_write == INDEX_STDOUT) // Xet truong hop file stdout
			{
				int pos = 0;
				while (buffer_write[pos] != '\0')
				{

					// write moi byte trong file ra sdt out
					// gSynchConsole->Write(buffer + pos, 1); // Su dung ham Write cua lop
					// SynchConsole
					kernel->synchConsoleOut->PutChar((buffer_write + pos)[0]);
					// den byte tiep theo
					pos++;
				}

				kernel->machine->WriteRegister(
					2, pos - 1); // Tra ve so byte thuc su write duoc
				delete buffer_write;
				increasePC();
				return;
			}
			else
			{
				// Xet truong hop ghi file read & write thi tra ve so byte thuc su
				if ((kernel->fileSystem->fileTable[id_write]->Write(buffer_write, charcount_write)) > 0)
				{

					// So byte thuc su = newPosition - curPosition
					newPosition_write = kernel->fileSystem->fileTable[id_write]->getCurrentOffset();
					kernel->machine->WriteRegister(2, newPosition_write - curPosition_write);
				}
				else
				{
					kernel->machine->WriteRegister(2, RW_END_FILE);
				}
			}
			delete buffer_write;
			increasePC();
			return;

		case SC_Seek:
			// Input: vi tri can di chuyen, id cua file
			// Output: loi: -1,
			// neu vi tri truyen vao: -1 => length of file
			// khac: tra ve vi tri da di chuyen den

			// Purpose: Di chuyen con tro file den 1 vi tri trong file
			int position_seek = kernel->machine->ReadRegister(4); // vi tri chuyen den
			int id_seek = kernel->machine->ReadRegister(5);		  // id file

			// Kiem tra id co nam ngoai bang mo ta file
			if (id_seek < 0 || id_seek >= MAX_FILE_OPEN)
			{
				printf("\nLoi di chuyen con tro vi id nam ngoai bang mo ta file.");
				kernel->machine->WriteRegister(2, -1);
				increasePC();
				return;
			}
			// Kiem tra file co ton tai khong
			if (kernel->fileSystem->fileTable[id_seek] == NULL)
			{
				printf("\nLoi di chuyen con tro vi file nay khong ton tai.");
				kernel->machine->WriteRegister(2, -1);
				increasePC();
				return;
			}
			// Seek voi cac file console
			if (id_seek == 0 || id_seek == 1)
			{
				printf("\nLoi di chuyen con tro tren file console.");
				kernel->machine->WriteRegister(2, -1);
				increasePC();
				return;
			}
			// position == -1, tra ve length
			if (position_seek == -1)
			{
				position_seek = kernel->fileSystem->fileTable[id_seek]->Length();
			}
			if (position_seek > kernel->fileSystem->fileTable[id_seek]->Length() ||
				position_seek < 0) // Kiem tra vi tri hop le
			{
				printf("\nLoi di chuyen con tro file den vi tri nay.");
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				// Di chuyen file thanh cong
				kernel->fileSystem->fileTable[id_seek]->Seek(position_seek);
				kernel->machine->WriteRegister(2, position_seek);
			}
			increasePC();
			return;
		}
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	ASSERTNOTREACHED();
}