#ifndef _TESTAPP_CPP
#define _TESTAPP_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS     //Disables strcpy() security warning on Microsoft compilers.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>
#include <StdLib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../CCRC32.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int iArg, char *sArg[])
{
	char sSourceFile[MAX_PATH];

	memset(sSourceFile, 0, sizeof(sSourceFile));

	if(iArg > 1)
	{
		//Get the passed in file name.
		strcpy(sSourceFile, sArg[1]);
	}
	else{
		//No file name was passed in, ask the user for the file name.
		printf("Enter a file name: ");
		gets(sSourceFile);
	}

	if(strlen(sSourceFile) > 0) //Do we have a file name?
	{
		CCRC32 crc32;
		crc32.Initialize(); //Only have to do this once.

		unsigned int iCRC = 0;

		if(crc32.FileCRC(sSourceFile, &iCRC))
		{
			printf("CRC32: %X\n", iCRC);
		}
		else{
			printf("File not found or access denied.\n");
		}
	}
	else{
		printf("No input file was specified.\n");
	}

	system("pause");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
