#include <Windows.h>
#include <iostream>

size_t fWriteByte(BYTE value, FILE *f);
size_t fWriteDword(DWORD value, FILE *f);
size_t fWriteFloat(float value, FILE *f);
size_t fWriteDouble(double value, FILE *f);