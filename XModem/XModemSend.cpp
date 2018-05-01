#include "stdafx.h"
#include <windows.h>
#include "iostream"
#include "string"
#include "CRCFunctions.h"
#include "XModemSend.h"

using namespace std;

const char SOH = 0x01;
const char NAK = 0x15;
const char CAN = 0x18;
const char ACK = 0x06;
const char EOT = 0x04;

int Send(LPCTSTR nazwaPortu)
{
	HANDLE   portHandle;
	DCB      controlSettings;
	COMSTAT commDeviceInfo;
	COMMTIMEOUTS timeParameters;
	DWORD   error;

	portHandle = CreateFile(nazwaPortu, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
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
		ClearCommError(portHandle, &error, &commDeviceInfo);
	}
	else 
	{
		cout << "Conection failed\n";
	}

	cout << "File name: ";
	char fileName[255];
	cin >> fileName;

	cout << "\nWaiting for transmittion start\n";

	char character;
	unsigned long characterSize = sizeof(character);
	int characterCount = 1;
	int kod;
	bool isTransmission = false;
	for (int i = 0; i < 6; i++)
	{

		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		cout << character << endl;
		if (character == 'C')
		{
			kod = 1;
			isTransmission = true;
			break;
		}
		else if (character == NAK)
		{
			kod = 2;
			isTransmission = true;
			break;
		}
	}


	if (!isTransmission)
	{
		system("PAUSE");
		return(0);
	}

	std::ifstream plik;
	plik.open(fileName, ios::binary);
	char packet[128];
	while (!plik.eof())
	{
		for (int i = 0; i < 128; i++)
		{
			packet[i] = (char)26;
		}
		int w = 0;

		while (w<128 && !plik.eof())
		{
			packet[w] = plik.get();
			if (plik.eof()) packet[w] = (char)26;
			w++;
		}
		bool isPacketCorrect = false;

		int packetNumber = 1;
		while (!isPacketCorrect)
		{
			cout << "Sending the packet\n";
			WriteFile(portHandle, &SOH, characterCount, &characterSize, NULL);		// send SOH
			character = (char)packetNumber;
			WriteFile(portHandle, &character, characterCount, &characterSize, NULL);		// send packet number
			character = (char)255 - packetNumber;
			WriteFile(portHandle, &character, characterCount, &characterSize, NULL); 		// complement


			for (int i = 0; i<128; i++)
				WriteFile(portHandle, &packet[i], characterCount, &characterSize, NULL);
			if (kod == 2)
			{
				char checksum = (char)26;
				for (int i = 0; i<128; i++)
					checksum += packet[i] % 256;
				WriteFile(portHandle, &checksum, characterCount, &characterSize, NULL);
				cout << " Checksum = " << checksum << endl;
			}
			else if (kod == 1)
			{
				USHORT tmpCRC = calculateCRC(packet, 128);
				character = calculateCharacterCRC(tmpCRC, 1);
				WriteFile(portHandle, &character, characterCount, &characterSize, NULL);
				character = calculateCharacterCRC(tmpCRC, 2);
				WriteFile(portHandle, &character, characterCount, &characterSize, NULL);
			}


			while (1)
			{
				character = ' ';
				ReadFile(portHandle, &character, characterCount, &characterSize, NULL);

				if (character == ACK)
				{
					isPacketCorrect = true;
					cout << "Successfully sent packet\n";
					break;
				}
				if (character == NAK)
				{
					cout << "Error, got NAK\n";
					break;
				}
				if (character == CAN)
				{
					cout << "Transmittion was interrupted\n";
					return 1;
				}
			}
		}
		if (packetNumber == 255) packetNumber = 1;
		else packetNumber++;

	}
	plik.close();

	while (1)
	{
		character = EOT;
		WriteFile(portHandle, &character, characterCount, &characterSize, NULL);
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		if (character == ACK) break;
	}

	CloseHandle(portHandle);
	cout << "File sent successfully\n";
}