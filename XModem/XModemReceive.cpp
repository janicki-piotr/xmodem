#include "stdafx.h"
#include <windows.h>
#include "iostream"
#include "string"
#include "Stale.h"
#include "CRCFunctions.h"
#include "XModemReceive.h"

using namespace std;

int Receive(LPCTSTR selectedPort)
{
	portHandle = CreateFile(selectedPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (portHandle != INVALID_HANDLE_VALUE)
	{
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

		timeParameters.ReadIntervalTimeout = 10000;
		timeParameters.ReadTotalTimeoutMultiplier = 10000;
		timeParameters.ReadTotalTimeoutConstant = 10000;
		timeParameters.WriteTotalTimeoutMultiplier = 100;
		timeParameters.WriteTotalTimeoutConstant = 100;

		SetCommState(portHandle, &controlSettings);
		SetCommTimeouts(portHandle, &timeParameters);
		ClearCommError(portHandle, &Error, &commDeviceInfo);
	}
	else {
		cout << "Conection failed\n";
		system("PAUSE");
		return 0;
	}

	char fileName[255];
	cout << "File name: ";
	cin >> fileName;
	cout << endl;

	int characterCount = 1;
	char character;
	unsigned long characterSize = sizeof(character);

	bool isTransmission = false;
	for (int i = 0; i < 6; i++)
	{
		character = 'C';
		WriteFile(portHandle, &character, characterCount, &characterSize, NULL);
		//czeka na SOH
		cout << "Waiting for SOH\n";
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		if (character == SOH)
		{
			isTransmission = true;
			break;
		}
	}
	if (!isTransmission)
	{
		cout << "Connection failed\n";
		system("PAUSE");
		return(0);
	}
	std::ofstream file;
	file.open(fileName, ios::binary);
	cout << "Receiving the file\n";

	ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
	int packageNumber = (int)character;

	ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
	char complementTo255 = character;

	char dataBlock[128];
	for (int i = 0; i<128; i++)
	{
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		dataBlock[i] = character;
	}

	char CRCChecksum[2];
	ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
	CRCChecksum[0] = character;
	ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
	CRCChecksum[1] = character;
	bool isPackageCorrect;

	if ((char)(255 - packageNumber) != complementTo255)
	{
		cout << "Bad package number\n";
		WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL);
		isPackageCorrect = false;

	}
	else
	{
		tmpCRC = PoliczCRC(dataBlock, 128);	// CRC

		if (PoliczCRC_Znaku(tmpCRC, 1) != CRCChecksum[0] || PoliczCRC_Znaku(tmpCRC, 2) != CRCChecksum[1])
		{
			cout << "Bad checksum\n";
			WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL); //NAK
			isPackageCorrect = false;
		}
		else
		{
			isPackageCorrect = true;
		}
	}

	if (isPackageCorrect)
	{
		for (int i = 0; i<128; i++)
		{
			if (dataBlock[i] != 26)
				file << dataBlock[i];
		}
		cout << "Package received successfully!\n";
		WriteFile(portHandle, &ACK, characterCount, &characterSize, NULL);
	}

	while (1)
	{
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		if (character == EOT || character == CAN) break;
		cout << "Transmittion in progress / ";

		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		packageNumber = (int)character;

		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		complementTo255 = character;

		for (int i = 0; i < 128; i++)
		{
			ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
			dataBlock[i] = character;
		}


		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		CRCChecksum[0] = character;
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		CRCChecksum[1] = character;
		isPackageCorrect = true;

		if ((char)(255 - packageNumber) != complementTo255)
		{
			cout << "Bad package number\n";
			WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL);
			isPackageCorrect = false;
		}
		else
		{
			tmpCRC = PoliczCRC(dataBlock, 128);

			if (PoliczCRC_Znaku(tmpCRC, 1) != CRCChecksum[0] || PoliczCRC_Znaku(tmpCRC, 2) != CRCChecksum[1])
			{
				cout << "Bad checksum\n";
				WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL);
				isPackageCorrect = false;
			}
		}
		if (isPackageCorrect)
		{
			for (int i = 0; i<128; i++)
			{
				if (dataBlock[i] != 26)
					file << dataBlock[i];
			}

			cout << "Package received successfully!\n";
			WriteFile(portHandle, &ACK, characterCount, &characterSize, NULL);
		}
	}
	WriteFile(portHandle, &ACK, characterCount, &characterSize, NULL);

	file.close();
	CloseHandle(portHandle);
	if (character == CAN)
	{
		cout << "Error, connection was interrupted\n";
	}
	else
	{
		cout << "Transmission completed\n";
	}
	system("Pause");
	return (0);
}