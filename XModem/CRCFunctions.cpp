#include "stdafx.h"
#include <iostream>
#include "CRCFunctions.h"

using namespace std;

HANDLE HandleConfig(LPCTSTR selectedPort)
{
	HANDLE portHandle = CreateFile(selectedPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (portHandle != INVALID_HANDLE_VALUE)
	{
		DCB controlSettings;
		controlSettings.DCBlength = sizeof(controlSettings);
		GetCommState(portHandle, &controlSettings);
		controlSettings.BaudRate = CBR_9600;
		controlSettings.Parity = NOPARITY;
		controlSettings.StopBits = ONESTOPBIT;
		controlSettings.ByteSize = 8;
		controlSettings.fParity = TRUE;
		controlSettings.fDtrControl = DTR_CONTROL_DISABLE;
		controlSettings.fRtsControl = RTS_CONTROL_DISABLE;

		COMMTIMEOUTS timeParameters;
		timeParameters.ReadIntervalTimeout = 5000;
		timeParameters.ReadTotalTimeoutMultiplier = 0;
		timeParameters.ReadTotalTimeoutConstant = 5000;
		timeParameters.WriteTotalTimeoutMultiplier = 0;
		timeParameters.WriteTotalTimeoutConstant = 5000;

		COMSTAT commDeviceInfo; DWORD error;
		SetCommState(portHandle, &controlSettings);
		SetCommTimeouts(portHandle, &timeParameters);
		ClearCommError(portHandle, &error, &commDeviceInfo);
		return (portHandle);
	}
	else
	{
		cout << "Conection failed\n";
		system("PAUSE");
		exit (0);
	}
}


int calculateCRC(char *wsk, int count)
{
	int ChecksumCRC = 0;
	while (--count >= 0) 
	{
		ChecksumCRC = ChecksumCRC ^ (int)*wsk++ << 8;
		for (int i = 0; i < 8; ++i)
		{
			if (ChecksumCRC & 0x8000)
				{ ChecksumCRC = ChecksumCRC << 1 ^ 0x1021; }
			else
				{ ChecksumCRC = ChecksumCRC << 1; }
		}
	}
	return (ChecksumCRC & 0xFFFF);
}

int checkIfEven(int x, int y)
{
	if (y == 0) { return 1; }
	if (y == 1) { return x; }

	int result = x;

	for (int i = 2; i <= y; i++)
		{ result = result * x; }

	return result;
}

char calculateCharacterCRC(int n, int characterNumber)
{
	int x, binary[16];

	for(int i = 0; i < 16; i++)
		{ binary[i] = 0; }

	for(int i = 0; i < 16; i++)
	{
		x = n % 2;
		if(x == 1) { n = (n - 1) / 2; }
		if(x == 0) { n = n / 2; }

		binary[15 - i] = x;
	}

	int result = 0;
	int k;

	if (characterNumber == 1) { k = 7; }
	if (characterNumber == 2) { k = 15; }

	for (int i = 0; i < 8; i++)
	{
		result += checkIfEven(2, i) * binary[k - i];
	}
	return (char)result;
}
