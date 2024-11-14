#include "FileManager.h"

size_t fWriteByte(BYTE value, FILE *f)
{
	return fwrite(&value, 1, 1, f);
}

size_t fWriteDword(DWORD value, FILE *f)
{
	return fwrite(&value, 4, 1, f);
}

size_t fWriteFloat(float value, FILE *f)
{
	return fwrite(&value, 4, 1, f);
}

size_t fWriteDouble(double value, FILE *f)
{
	return fwrite(&value, sizeof(value), 1, f);
}