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

int characterCount = 1; unsigned long characterSize = sizeof(char);

int Send(LPCTSTR selectedPort)
{
	HANDLE portHandle = HandleConfig(selectedPort); //Handle configuration

	cout << "File name: ";
	char fileName[255];
	cin >> fileName;
	cout << endl;

	cout << "Waiting for transmittion." << endl;
	char character;
	bool variant;
	bool isTransmittion = false;
	for (int i = 0; i < 6; i++)
	{
		ReadFile(portHandle, &character, characterCount, &characterSize, NULL); //Waiting for transmittion start
		if (character == 'C')//Got C character, CRC variant
		{
			variant = true;
			isTransmittion = true;
			break;
		}
		else if (character == NAK)//Got NAK, checksum variant
		{
			variant = false;
			isTransmittion = true;
			break;
		}
	}

	if (!isTransmittion)
	{
		cout << "Transmition failed" << endl;
		system("PAUSE");
		return (0);
	}

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

		int i = 0;
		while (i < 128 && !file.eof())
		{
			packet[i] = file.get();
			if ((file.eof()))
			{
				packet[i] = (char)26;
			}
			i++;
		}
		bool isPacketCorrect = false;

		while (!isPacketCorrect)
		{
			cout << "Sending the packet" << endl;
			WriteFile(portHandle, &SOH, characterCount, &characterSize, NULL);//Sending SOH
			character = (char)packetNumber;
			WriteFile(portHandle, &character, characterCount, &characterSize, NULL);//Sending packet number
			character = (char)255 - packetNumber;
			WriteFile(portHandle, &character, characterCount, &characterSize, NULL);//Sending 255 - packet number


			for (int i = 0; i < 128; i++)
			{
				WriteFile(portHandle, &packet[i], characterCount, &characterSize, NULL);
			}
			if (!variant) //checksum
			{
				char checksum = 0;
				for (int i = 0; i < 128; i++)
					{ checksum += packet[i] % 256; }
				WriteFile(portHandle, &checksum, characterCount, &characterSize, NULL);
			}
			else if (variant) //checksum CRC
			{
				int CRC = calculateCRC(packet, 128);
				character = calculateCharacterCRC(CRC, 1);
				WriteFile(portHandle, &character, characterCount, &characterSize, NULL);
				character = calculateCharacterCRC(CRC, 2);
				WriteFile(portHandle, &character, characterCount, &characterSize, NULL);
			}

			while (1)
			{
				character = ' ';
				ReadFile(portHandle, &character, characterCount, &characterSize, NULL);//Waiting for NAK, CAN or ACK

				if (character == ACK) //Everything ok
				{
					isPacketCorrect = true;
					cout << "Packet transmitted successfully" << endl;
					break;
				}
				if (character == NAK) // Error with checksum
				{
					isPacketCorrect = true;
					cout << "Checksum error" << endl;
					break;
				}
				if (character == CAN) // Sth went wrong
				{
					cout << "Transmition failed" << endl;
					system("PAUSE");
					return (0);
				}
			}
		}
		if (packetNumber < 255) { packetNumber++; }
		else { packetNumber = 1; } // next packet inc., increasing the number

	}
	file.close();

	while (1) //closing the transmittion, sending EOT and waiting for ACK
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