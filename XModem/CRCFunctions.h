#pragma once
#include <fstream>
#include <string.h>
#include <windows.h>

int calculateCRC(char*, int);
int checkIfEven(int, int);
char calculateCharacterCRC(int, int);
HANDLE HandleConfig(LPCTSTR);