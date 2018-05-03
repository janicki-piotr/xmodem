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
const char C = 0x43;

int Send(LPCTSTR selectedPort)
{
	HANDLE portHandle = HandleConfig(selectedPort);

	cout << "File name: ";
	char fileName[255];
	cin >> fileName;
	cout << endl;

	cout << "Waiting for transmittion." << endl;
	char character; int characterCount = 1; unsigned long characterSize = sizeof(character);
	int kod;
	bool isTransmittion = false;
	for (int i = 0; i < 6; i++)
	{
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		if (character == 'C')
		{
			kod = 1;
			isTransmittion = true;
			break;
		}
		else if (character == NAK)
		{
			kod = 2;
			isTransmittion = true;
			break;
		}
	}

	if (!isTransmittion) return(0);

	ifstream file;
	file.open(fileName, ios::binary);

	int packetNumber = 1;
	char packet[128];
	while (!file.eof())
	{
		for (int i = 0; i < 128; i++)
		{
			packet[i] = (char)26;
		}
		int w = 0;

		while (w < 128 && !file.eof())
		{
			packet[w] = file.get();
			if (file.eof()) packet[w] = (char)26;
			w++;
		}
		bool isPacketCorrect = false;

		while (!isPacketCorrect)
		{
			cout << "Sending the packet" << endl;
			WriteFile(portHandle, &SOH, characterCount, &characterSize, NULL);		// wysy³anie SOH
			character = (char)packetNumber;
			WriteFile(portHandle, &character, characterCount, &characterSize, NULL);		// wys³anie numeru paczki
			character = (char)255 - packetNumber;
			WriteFile(portHandle, &character, characterCount, &characterSize, NULL); 		// wys³anie dope³nienia


			for (int i = 0; i < 128; i++)
			{
				WriteFile(portHandle, &packet[i], characterCount, &characterSize, NULL);
			}
			if (kod == 2) //checksum
			{
				char checksum = 0;
				for (int i = 0; i < 128; i++)
					{ checksum += packet[i] % 256; }
				WriteFile(portHandle, &checksum, characterCount, &characterSize, NULL);
			}
			else if (kod == 1) //checksum CRC
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
					cout << "Packet transmitted successfully" << endl;
					break;
				}
				if (character == NAK)
				{
					isPacketCorrect = true;
					cout << "Checksum error" << endl;
					break;
				}
				if (character == CAN)
				{
					cout << "Transmition failed" << endl;
					system("PAUSE");
					return (0);
				}
			}
		}
		if (packetNumber == 255) { packetNumber = 1; }
		else { packetNumber++; }

	}
	file.close();

	while (1)
	{
		character = EOT;
		WriteFile(portHandle, &character, characterCount, &characterSize, NULL);
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL);
		if (character == ACK) { break; }
	}

	CloseHandle(portHandle);
	cout << endl << "Transmission completed" << endl;
	system("PAUSE");
	return (0);

}