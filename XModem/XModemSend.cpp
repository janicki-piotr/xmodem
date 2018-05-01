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

HANDLE   uchwytPortu;                      	// identyfikator portu
LPCTSTR  nazwaPortu;                    	// przechowuje nazwê portu
DCB      ustawieniaSterowania;              // struktura kontroli portu szeregowego
COMSTAT zasobyPortu;                        // dodatkowa informacja o zasobach portu
DWORD   blad;                         	    // reprezentuje typ ewentualnego b³êdu
COMMTIMEOUTS ustawieniaCzasu;
USHORT tmpCRC;

char nazwaPliku[255];                   // bufor na nazwe pliku

char znak;                              // bufor na przesylany znak
int licznikZnakow = 1;
unsigned long rozmiarZnaku = sizeof(znak);
int kod;

bool transmisja = false;
bool czyPoprawnyPakiet;
int nrPakietu = 1;
char paczka[128];

int Send(LPCTSTR nazwaPortu)
{
	std::ifstream plik;

	//ustawiamy parametry portu i go otwieramy
	uchwytPortu = CreateFile(nazwaPortu, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (uchwytPortu != INVALID_HANDLE_VALUE)
	{
		ustawieniaSterowania.DCBlength = sizeof(ustawieniaSterowania);
		GetCommState(uchwytPortu, &ustawieniaSterowania);
		ustawieniaSterowania.BaudRate = CBR_9600; 				// prêdkosæ transmisji
		ustawieniaSterowania.Parity = NOPARITY;   				// bez bitu parzystoœci
		ustawieniaSterowania.StopBits = ONESTOPBIT; 			// ustawienie bitu stopu (jeden bit)
		ustawieniaSterowania.ByteSize = 8;  					// liczba wysy³anych bitów

		ustawieniaSterowania.fParity = TRUE;
		ustawieniaSterowania.fDtrControl = DTR_CONTROL_DISABLE; //Kontrola linii DTR: DTR_CONTROL_DISABLE (sygna³ nieaktywny)
		ustawieniaSterowania.fRtsControl = RTS_CONTROL_DISABLE; //Kontrola linii RTR: DTR_CONTROL_DISABLE (sygna³ nieaktywny)
		ustawieniaSterowania.fOutxCtsFlow = FALSE;
		ustawieniaSterowania.fOutxDsrFlow = FALSE;
		ustawieniaSterowania.fDsrSensitivity = FALSE;
		ustawieniaSterowania.fAbortOnError = FALSE;
		ustawieniaSterowania.fOutX = FALSE;
		ustawieniaSterowania.fInX = FALSE;
		ustawieniaSterowania.fErrorChar = FALSE;
		ustawieniaSterowania.fNull = FALSE;

		ustawieniaCzasu.ReadIntervalTimeout = 10000;
		ustawieniaCzasu.ReadTotalTimeoutMultiplier = 10000;
		ustawieniaCzasu.ReadTotalTimeoutConstant = 10000;
		ustawieniaCzasu.WriteTotalTimeoutMultiplier = 100;
		ustawieniaCzasu.WriteTotalTimeoutConstant = 100;

		SetCommState(uchwytPortu, &ustawieniaSterowania);
		SetCommTimeouts(uchwytPortu, &ustawieniaCzasu);
		ClearCommError(uchwytPortu, &blad, &zasobyPortu);
	}
	else {
		cout << "Nieudane polaczenie (COM1, 9600kb/s, 8-bitowe dane, jeden bit stopu)\n";
	}

	cout << "Nazwa pliku do WYSLANIA: ";
	cin >> nazwaPliku;

	cout << "\nOczekiwanie na rozpoczecie transmisji...\n";
	for (int i = 0; i<6; i++)
	{

		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		cout << znak << endl;
		if (znak == 'C')
		{
			cout << " | otrzymano znak\n......" << znak << endl;;
			kod = 1;
			transmisja = true;
			break;
		}
		else if (znak == NAK)
		{
			cout << " | otrzymano NAK\n";
			kod = 2;
			transmisja = true;
			break;

		}
	}


	if (!transmisja) exit(1);

	plik.open(nazwaPliku, ios::binary);
	while (!plik.eof())
	{
		//tablica do czyszczenia
		for (int i = 0; i<128; i++)
			paczka[i] = (char)26;

		int w = 0;


		while (w<128 && !plik.eof())
		{
			paczka[w] = plik.get();
			if (plik.eof()) paczka[w] = (char)26;
			w++;
		}
		czyPoprawnyPakiet = false;

		while (!czyPoprawnyPakiet)
		{
			cout << "Trwa wysylanie pakietu. Prosze czekac...\n";
			WriteFile(uchwytPortu, &SOH, licznikZnakow, &rozmiarZnaku, NULL);		// wysy³anie SOH
			znak = (char)nrPakietu;
			WriteFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);		// wys³anie numeru paczki
			znak = (char)255 - nrPakietu;
			WriteFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL); 		// wys³anie dope³nienia


			for (int i = 0; i<128; i++)
				WriteFile(uchwytPortu, &paczka[i], licznikZnakow, &rozmiarZnaku, NULL);
			if (kod == 2) //suma kontrolna
			{
				char suma_kontrolna = (char)26;
				for (int i = 0; i<128; i++)
					suma_kontrolna += paczka[i] % 256;
				WriteFile(uchwytPortu, &suma_kontrolna, licznikZnakow, &rozmiarZnaku, NULL);
				cout << " Suma kontrolna = " << suma_kontrolna << endl;
			}
			else if (kod == 1) //obliczanie CRC i transfer
			{
				tmpCRC = PoliczCRC(paczka, 128);
				znak = PoliczCRC_Znaku(tmpCRC, 1);
				WriteFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
				znak = PoliczCRC_Znaku(tmpCRC, 2);
				WriteFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
			}


			while (1)
			{
				znak = ' ';
				ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);

				if (znak == ACK)
				{
					czyPoprawnyPakiet = true;
					cout << "Przeslano poprawnie pakiet danych!";
					break;
				}
				if (znak == NAK)
				{
					cout << "ERROR - otrzymano NAK!\n";
					break;
				}
				if (znak == CAN)
				{
					cout << "ERROR - polaczenie zostalo przerwane!\n";
					return 1;
				}
			}
		}
		//zwiekszamy numer pakietu
		if (nrPakietu == 255) nrPakietu = 1;
		else nrPakietu++;

	}
	plik.close();

	while (1)
	{
		znak = EOT;
		WriteFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		if (znak == ACK) break;
	}

	CloseHandle(uchwytPortu);
	cout << "Hurra! Udalo sie wyslac plik!";

}