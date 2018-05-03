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
		controlSettings.fOutxCtsFlow = FALSE;
		controlSettings.fOutxDsrFlow = FALSE;
		controlSettings.fDsrSensitivity = FALSE;
		controlSettings.fAbortOnError = FALSE;
		controlSettings.fOutX = FALSE;
		controlSettings.fInX = FALSE;
		controlSettings.fErrorChar = FALSE;
		controlSettings.fNull = FALSE;

		COMMTIMEOUTS timeParameters;
		timeParameters.ReadIntervalTimeout = 10000;
		timeParameters.ReadTotalTimeoutMultiplier = 10000;
		timeParameters.ReadTotalTimeoutConstant = 10000;
		timeParameters.WriteTotalTimeoutMultiplier = 100;
		timeParameters.WriteTotalTimeoutConstant = 100;

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
		ChecksumCRC = ChecksumCRC ^ (int)*wsk++ << 8; 								 // weŸ znak i dopisz osiem zer
		for (int i = 0; i < 8; ++i)
		{
			if (ChecksumCRC & 0x8000)
			{
				ChecksumCRC = ChecksumCRC << 1 ^ 0x1021;       // jeœli lewy bit == 1 wykonuj XOR generatorm 1021
			}
			else
			{
				ChecksumCRC = ChecksumCRC << 1; 									 // jeœli nie to XOR przez 0000, czyli przez to samo
			}
		}
	}
	return (ChecksumCRC & 0xFFFF);
}

int checkIfEven(int x, int y)
{
	if (y == 0) { return 1; }
	if (y == 1) { return x; }

	int wynik = x;

	for (int i = 2; i <= y; i++)
	{
		wynik = wynik * x;
	}

	return wynik;
}

char calculateCharacterCRC(int n, int nrZnaku) //przeliczanie CRC na postaæ binarn¹
{
	int x, binary[16];

	for (int i = 0; i < 16; i++)
	{
		binary[i] = 0;
	}

	for (int i = 0; i < 16; i++)
	{
		x = n % 2;
		if (x == 1) n = (n - 1) / 2;
		if (x == 0) n = n / 2;
		binary[15 - i] = x;
	}

	//obliczamy poszczegolne znaki sumaKontrolnaCRC (1-szy lub 2-gi)
	x = 0;
	int k;

	if (nrZnaku == 1) { k = 7; }
	if (nrZnaku == 2) { k = 15; }

	for (int i = 0; i < 8; i++)
	{
		x = x + checkIfEven(2, i) * binary[k - i];
	}
	return (char)x;//zwraca 1 lub 2 znak (bo 2 znaki to 2 bajty, czyli 16 bitów)
}
