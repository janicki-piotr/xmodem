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

ifstream plik;
char nazwaPliku[255];                   // bufor na nazwe pliku

char znak;                              // bufor na przesylany znak
int licznikZnakow = 1;
unsigned long rozmiarZnaku = sizeof(znak);
int kod;

bool transmisja = false;
bool czyPoprawnyPakiet;
int nrPakietu = 1;
char paczka[128];

int Send(LPCTSTR selectedPort)
{
	HANDLE   portHandle;            // Handle for a port
	DCB      controlSettings;       // Defines the control setting for a serial communications device.
	COMSTAT	 commDeviceInfo;        // Contains information about a communications device. This structure is filled by the ClearCommError function
	COMMTIMEOUTS timeParameters;	// Contains the time-out parameters for a communications device. 
	DWORD    error;
	USHORT tmpCRC;

	portHandle = CreateFile(selectedPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (portHandle != INVALID_HANDLE_VALUE)
	{
		controlSettings.DCBlength = sizeof(controlSettings);
		GetCommState(portHandle, &controlSettings);
		controlSettings.BaudRate = CBR_9600; 				// prêdkosæ transmisji
		controlSettings.Parity = NOPARITY;   				// bez bitu parzystoœci
		controlSettings.StopBits = ONESTOPBIT; 			// ustawienie bitu stopu (jeden bit)
		controlSettings.ByteSize = 8;  					// liczba wysy³anych bitów

		controlSettings.fParity = TRUE;
		controlSettings.fDtrControl = DTR_CONTROL_DISABLE; //Kontrola linii DTR: DTR_CONTROL_DISABLE (sygna³ nieaktywny)
		controlSettings.fRtsControl = RTS_CONTROL_DISABLE; //Kontrola linii RTR: DTR_CONTROL_DISABLE (sygna³ nieaktywny)
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
	else {
		cout << "Conection failed" << endl;
	}

	cout << "File name: ";
	cin >> nazwaPliku;
	cout << endl;

	cout << "Waiting for transmittion." << endl;
	for (int i = 0; i < 6; i++)
	{

		ReadFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		if (znak == 'C')
		{
			kod = 1;
			transmisja = true;
			break;
		}
		else if (znak == NAK)
		{
			kod = 2;
			transmisja = true;
			break;
		}
	}

	if (!transmisja) return(0);

	plik.open(nazwaPliku, ios::binary);
	while (!plik.eof())
	{
		for (int i = 0; i < 128; i++)
		{
			paczka[i] = (char)26;
		}
		int w = 0;

		while (w < 128 && !plik.eof())
		{
			paczka[w] = plik.get();
			if (plik.eof()) paczka[w] = (char)26;
			w++;
		}
		czyPoprawnyPakiet = false;

		while (!czyPoprawnyPakiet)
		{
			cout << "Sending the packet" << endl;
			WriteFile(portHandle, &SOH, licznikZnakow, &rozmiarZnaku, NULL);		// wysy³anie SOH
			znak = (char)nrPakietu;
			WriteFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL);		// wys³anie numeru paczki
			znak = (char)255 - nrPakietu;
			WriteFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL); 		// wys³anie dope³nienia


			for (int i = 0; i < 128; i++)
			{
				WriteFile(portHandle, &paczka[i], licznikZnakow, &rozmiarZnaku, NULL);
			}
			if (kod == 2) //suma kontrolna
			{
				char suma_kontrolna = 0;
				for (int i = 0; i < 128; i++)
				{
					suma_kontrolna += paczka[i] % 256;
				}
				WriteFile(portHandle, &suma_kontrolna, licznikZnakow, &rozmiarZnaku, NULL);
			}
			else if (kod == 1) //obliczanie CRC i transfer
			{
				tmpCRC = calculateCRC(paczka, 128);
				znak = calculateCharacterCRC(tmpCRC, 1);
				WriteFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL);
				znak = calculateCharacterCRC(tmpCRC, 2);
				WriteFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL);
			}

			while (1)
			{
				znak = ' ';
				ReadFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL);

				if (znak == ACK)
				{
					czyPoprawnyPakiet = true;
					cout << "Packet transmitted successfully" << endl;
					break;
				}
				if (znak == NAK)
				{
					czyPoprawnyPakiet = true;
					cout << "Checksum error" << endl;
					break;
				}
				if (znak == CAN)
				{
					cout << "Transmition failed" << endl;
					return 1;
				}
			}
		}
		if (nrPakietu == 255) { nrPakietu = 1; }
		else { nrPakietu++; }

	}
	plik.close();

	while (1)
	{
		znak = EOT;
		WriteFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		ReadFile(portHandle, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		if (znak == ACK) { break; }
	}

	CloseHandle(portHandle);
	cout << "Hurra! Udalo sie wyslac plik!";
	return 0;

}