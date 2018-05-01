#include "stdafx.h"
#include "CRCFunctions.h"

using namespace std;


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
