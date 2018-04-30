#include "stdafx.h"
#include <windows.h>
#include "iostream"
#include "string"
#include "Stale.h"
#include "CRCFunctions.h"
#include "XModemReceive.h"

using namespace std;

void Receive(LPCTSTR nazwaPortu)
{
	std::ofstream plik;
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

	cout << "Nazwa pliku do ZAPISU: ";
	cin >> nazwaPliku;

	for (int i = 0; i<6; i++)
	{
		cout << "\nWysylanie\n";
		znak = 'C';
		WriteFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		//czeka na SOH
		cout << "Oczekiwanie na komunikat SOH...\n";
		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		cout << znak << endl;
		if (znak == SOH)
		{
			cout << "Ustanowienie polaczenia powiodlo sie!\n";
			transmisja = true;
			break;
		}
	}
	//nie nadszedl SOH
	if (!transmisja)
	{
		cout << "ERROR - polaczenie nieudane\n";
		exit(1);
	}
	plik.open(nazwaPliku, ios::binary);
	cout << "Trwa odbieranie pliku, prosze czekac...";

	ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
	numerPaczki = (int)znak;

	ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
	dopelnienieDo255 = znak;

	for (int i = 0; i<128; i++)
	{
		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		blokDanych[i] = znak;
	}

	ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
	sumaKontrolnaCRC[0] = znak;
	ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
	sumaKontrolnaCRC[1] = znak;
	poprawnyPakiet = true;


	if ((char)(255 - numerPaczki) != dopelnienieDo255)
	{
		cout << "ERROR - otrzymano niepoprawny numer pakietu!\n";
		WriteFile(uchwytPortu, &NAK, licznikZnakow, &rozmiarZnaku, NULL);
		poprawnyPakiet = false;

	}
	else
	{
		tmpCRC = PoliczCRC(blokDanych, 128);	// sprawdzanie czy sumy kontrole s¹ poprawne

		if (PoliczCRC_Znaku(tmpCRC, 1) != sumaKontrolnaCRC[0] || PoliczCRC_Znaku(tmpCRC, 2) != sumaKontrolnaCRC[1])
		{
			cout << "ERROR - zla suma kontrola!\n";
			WriteFile(uchwytPortu, &NAK, licznikZnakow, &rozmiarZnaku, NULL); //NAK
			poprawnyPakiet = false;
		}
	}

	if (poprawnyPakiet)
	{
		for (int i = 0; i<128; i++)
		{
			if (blokDanych[i] != 26)
				plik << blokDanych[i];
		}
		cout << "Przeslanie pakietu zakonczone powodzeniem!\n";
		WriteFile(uchwytPortu, &ACK, licznikZnakow, &rozmiarZnaku, NULL);
	}

	while (1)
	{
		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		if (znak == EOT || znak == CAN) break;
		cout << "Trwa odbieranie danych...";

		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		numerPaczki = (int)znak;

		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		dopelnienieDo255 = znak;

		for (int i = 0; i<128; i++)
		{
			ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
			blokDanych[i] = znak;
		}


		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		sumaKontrolnaCRC[0] = znak;
		ReadFile(uchwytPortu, &znak, licznikZnakow, &rozmiarZnaku, NULL);
		sumaKontrolnaCRC[1] = znak;
		poprawnyPakiet = true;

		if ((char)(255 - numerPaczki) != dopelnienieDo255)
		{
			cout << "ERROR - zly numer pakietu!\n";
			WriteFile(uchwytPortu, &NAK, licznikZnakow, &rozmiarZnaku, NULL);
			poprawnyPakiet = false;
		}
		else
		{
			tmpCRC = PoliczCRC(blokDanych, 128);

			if (PoliczCRC_Znaku(tmpCRC, 1) != sumaKontrolnaCRC[0] || PoliczCRC_Znaku(tmpCRC, 2) != sumaKontrolnaCRC[1])
			{
				cout << "ERROR - zla suma kontrolna!\n";
				WriteFile(uchwytPortu, &NAK, licznikZnakow, &rozmiarZnaku, NULL);
				poprawnyPakiet = false;
			}
		}
		if (poprawnyPakiet)
		{
			for (int i = 0; i<128; i++)
			{
				if (blokDanych[i] != 26)
					plik << blokDanych[i];
			}

			cout << "Przeslanie pakietu zakonczone powodzeniem!\n";
			WriteFile(uchwytPortu, &ACK, licznikZnakow, &rozmiarZnaku, NULL);
		}
	}
	WriteFile(uchwytPortu, &ACK, licznikZnakow, &rozmiarZnaku, NULL);

	plik.close();
	CloseHandle(uchwytPortu);

	if (znak == CAN) cout << "ERROR - polaczenie zostalo przerwane! \n";
	else cout << "Hurra! Plik w calosci odebrany!";
	cin.get();
	cin.get();
	int a;
	cin >> a;



}