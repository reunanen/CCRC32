///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCRC32_CPP
#define _CCRC32_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS     //Disables fopen() security warning on Microsoft compilers.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>

#include "CCRC32.H"

#ifdef _USE_GLOBAL_MEMPOOL
#include "../CMemPool/CMemPool.H"
extern CMemPool *pMem; //pMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function initializes "CRC Lookup Table". You only need to call it once to
		initalize the table before using any of the other CRC32 calculation functions.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCRC32::CCRC32(void)
{
    this->Initialize();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCRC32::~CCRC32(void)
{
    //No destructor code.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function initializes "CRC Lookup Table". You only need to call it once to
		initalize the table before using any of the other CRC32 calculation functions.
*/

void CCRC32::Initialize(void)
{
	//0x04C11DB7 is the official polynomial used by PKZip, WinZip and Ethernet.
	unsigned int ulPolynomial = 0x04C11DB7;

	memset(&this->ulTable, 0, sizeof(this->ulTable));

	// 256 values representing ASCII character codes.
	for(int iCodes = 0; iCodes <= 0xFF; iCodes++)
	{
		this->ulTable[iCodes] = this->Reflect(iCodes, 8) << 24;

		for(int iPos = 0; iPos < 8; iPos++)
		{
			this->ulTable[iCodes] = (this->ulTable[iCodes] << 1)
				^ ((this->ulTable[iCodes] & (1 << 31)) ? ulPolynomial : 0);
		}

		this->ulTable[iCodes] = this->Reflect(this->ulTable[iCodes], 32);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Reflection is a requirement for the official CRC-32 standard.
	You can create CRCs without it, but they won't conform to the standard.
*/

unsigned int CCRC32::Reflect(unsigned int ulReflect, const char cChar)
{
	unsigned int ulValue = 0;

	// Swap bit 0 for bit 7, bit 1 For bit 6, etc....
	for(int iPos = 1; iPos < (cChar + 1); iPos++)
	{
		if(ulReflect & 1)
		{
			ulValue |= (1 << (cChar - iPos));
		}
		ulReflect >>= 1;
	}

	return ulValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 by looping through each of the bytes in sData.
	
	Note: For Example usage example, see FileCRC().
*/

void CCRC32::PartialCRC(unsigned int *ulCRC, const unsigned char *sData, size_t ulDataLength)
{
	while(ulDataLength--)
	{
		//If your compiler complains about the following line, try changing
		//	each occurrence of *ulCRC with ((unsigned int)*ulCRC).

		*ulCRC = (*ulCRC >> 8) ^ this->ulTable[(*ulCRC & 0xFF) ^ *sData++];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the calculated CRC32 (through ulOutCRC) for the given string.
*/

void CCRC32::FullCRC(const unsigned char *sData, size_t ulDataLength, unsigned int *ulOutCRC)
{
    ((unsigned int)*ulOutCRC) = 0xffffffff; //Initilaize the CRC.

	this->PartialCRC(ulOutCRC, sData, ulDataLength);

	((unsigned int)*ulOutCRC) ^= 0xffffffff; //Finalize the CRC.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the calculated CRC23 for the given string.
*/

unsigned int CCRC32::FullCRC(const unsigned char *sData, size_t ulDataLength)
{
    unsigned int ulCRC = 0xffffffff; //Initilaize the CRC.

	this->PartialCRC(&ulCRC, sData, ulDataLength);

	return(ulCRC ^ 0xffffffff); //Finalize the CRC and return.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a user defined buffer.

	Note: The buffer size DOES NOT affect the resulting CRC,
			it has been provided for performance purposes only.
*/

bool CCRC32::FileCRC(const char *sFileName, unsigned int *ulOutCRC, size_t ulBufferSize)
{
    ((unsigned int)*ulOutCRC) = 0xffffffff; //Initilaize the CRC.

	FILE *fSource = NULL;
	unsigned char *sBuf = NULL;
	size_t iBytesRead = 0;

	if((fSource = fopen(sFileName, "rb")) == NULL)
	{
		return false; //Failed to open file for read access.
	}

	#ifdef _USE_GLOBAL_MEMPOOL
		if(!(sBuf = (unsigned char *)pMem->Allocate(ulBufferSize, 1))) //Allocate memory for file buffering.
	#else
		if(!(sBuf = (unsigned char *)malloc(ulBufferSize))) //Allocate memory for file buffering.
	#endif
	{
		fclose(fSource);
		return false; //Out of memory.
	}

	while((iBytesRead = fread(sBuf, sizeof(char), ulBufferSize, fSource)))
	{
		this->PartialCRC(ulOutCRC, sBuf, iBytesRead);
	}

	#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sBuf);
	#else
		free(sBuf);
	#endif

	fclose(fSource);

	((unsigned int)*ulOutCRC) ^= 0xffffffff; //Finalize the CRC.

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a default buffer size of 1MB.
*/

unsigned int CCRC32::FileCRC(const char *sFileName)
{
	unsigned int ulCRC;
	if(this->FileCRC(sFileName, &ulCRC, 1048576))
	{
		return ulCRC;
	}
	else return 0xffffffff; //While we return this as an error code, it is infact a valid CRC!
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a default buffer size of 1MB.

	Note: The buffer size DOES NOT affect the resulting CRC,
			it has been provided for performance purposes only.
*/

unsigned int CCRC32::FileCRC(const char *sFileName, size_t ulBufferSize)
{
	unsigned int ulCRC;
	if(this->FileCRC(sFileName, &ulCRC, ulBufferSize))
	{
		return ulCRC;
	}
	else return 0xffffffff; //While we return this as an error code, it is infact a valid CRC!
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Calculates the CRC32 of a file using the a default buffer size of 1MB.
*/

bool CCRC32::FileCRC(const char *sFileName, unsigned int *ulOutCRC)
{
	return this->FileCRC(sFileName, ulOutCRC, 1048576);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
