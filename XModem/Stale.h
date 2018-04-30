#pragma once
const char SOH= 0x01;
const char NAK= 0x15;
const char CAN= 0x18;
const char ACK= 0x06;
const char EOT= 0x04;

HANDLE   uchwytPortu;                      	// identyfikator portu
LPCTSTR  nazwaPortu;                    	// przechowuje nazwê portu
DCB      ustawieniaSterowania;              // struktura kontroli portu szeregowego
COMSTAT zasobyPortu;                        // dodatkowa informacja o zasobach portu
DWORD   blad;                         	    // reprezentuje typ ewentualnego b³êdu
COMMTIMEOUTS ustawieniaCzasu;
USHORT tmpCRC;

char PoliczCRC_Znaku(int n, int nrZnaku);
int czyParzysty(int x, int y);
int PoliczCRC(char *wsk, int count);
