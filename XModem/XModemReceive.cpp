#include "stdafx.h"
#include <windows.h>
#include "iostream"
#include "string"
#include "CRCFunctions.h"
#include "XModemReceive.h"

using namespace std;

const char SOH = 0x01;
const char NAK = 0x15;
const char CAN = 0x18;
const char ACK = 0x06;
const char EOT = 0x04;
const char C = 0x43;



int Receive(LPCTSTR selectedPort)
{
	int characterCount = 1; char character;
	unsigned long characterSize = sizeof(char);

	HANDLE portHandle = HandleConfig(selectedPort);

	char fileName[255];
	cout << "File name: ";
	cin >> fileName;
	cout << endl;

	bool isTransmission = false;
	for (int i = 0; i < 6; i++)
	{
		character = C;
		WriteFile(portHandle, &character, characterCount, &characterSize, NULL);// Sending C
		cout << "Waiting for SOH\n";
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);// Waiting for SOH
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

	ReadFile(portHandle, &character, characterCount, &characterSize, NULL); // Get packet number
	int packetNumber = (int)character;

	ReadFile(portHandle, &character, characterCount, &characterSize, NULL); // Get complement 255 - packet
	char complementTo255 = character;

	char dataBlock[128];
	for (int i = 0; i<128; i++)
	{
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL); // Get data
		dataBlock[i] = character;
	}

	char CRCChecksum[2];
	ReadFile(portHandle, &character, characterCount, &characterSize, NULL); // Get CRC
	CRCChecksum[0] = character;
	ReadFile(portHandle, &character, characterCount, &characterSize, NULL); // Get CRC
	CRCChecksum[1] = character;
	bool isPacketCorrect;

	if ((char)(255 - packetNumber) != complementTo255) // check checksum
	{
		cout << "Bad packet number\n";
		WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL);//if wrong, send NAK
		isPacketCorrect = false;

	}
	else
	{
		USHORT tmpCRC = calculateCRC(dataBlock, 128);

		if (calculateCharacterCRC(tmpCRC, 1) != CRCChecksum[0] || calculateCharacterCRC(tmpCRC, 2) != CRCChecksum[1]) // check CRC
		{
			cout << "Bad checksum\n";
			WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL);//if wrong, send NAK
			isPacketCorrect = false;
		}
		else
		{
			isPacketCorrect = true; // if everything is ok
		}
	}

	if (isPacketCorrect)
	{
		for (int i = 0; i<128; i++)
		{
			if (dataBlock[i] != 26)
				file << dataBlock[i];
		}
		cout << "Packet received successfully!\n";
		WriteFile(portHandle, &ACK, characterCount, &characterSize, NULL);
	}

	while (1)
	{
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		if (character == EOT || character == CAN) break;
		cout << "Transmittion in progress / ";

		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		packetNumber = (int)character;

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
		isPacketCorrect = true;

		if ((char)(255 - packetNumber) != complementTo255)
		{
			cout << "Bad packet number\n";
			WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL);
			isPacketCorrect = false;
		}
		else
		{
			USHORT tmpCRC = calculateCRC(dataBlock, 128);

			if (calculateCharacterCRC(tmpCRC, 1) != CRCChecksum[0] || calculateCharacterCRC(tmpCRC, 2) != CRCChecksum[1])
			{
				cout << "Bad checksum\n";
				WriteFile(portHandle, &NAK, characterCount, &characterSize, NULL);
				isPacketCorrect = false;
			}
		}
		if (isPacketCorrect)
		{
			for (int i = 0; i<128; i++)
			{
				if (dataBlock[i] != 26)
					file << dataBlock[i];
			}

			cout << "Packet received successfully!\n";
			WriteFile(portHandle, &ACK, characterCount, &characterSize, NULL);
		}
	}
	WriteFile(portHandle, &ACK, characterCount, &characterSize, NULL); // accept transmission end

	file.close();
	CloseHandle(portHandle);
	if (character == CAN)
	{
		cout << "Connection was interrupted\n";
	}
	else
	{
		cout << "Transmission completed\n";
	}
	system("Pause");
	return (0);
}