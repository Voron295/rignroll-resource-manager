#ifndef TARGA_CLASS
#define TARGA_CLASS

#include <Windows.h>
#include <iostream>

#define IMAGE_RGB_565 1
#define IMAGE_RGBA_4444 2
#define IMAGE_RGB_888 3
#define IMAGE_RGBA_8888 4

#define SOURCE_FILE 1
#define SOURCE_MEMORY 2

#pragma pack(push, 1)
typedef struct {
	char  idlength;
	char  colourmaptype;
	char  datatypecode;
	short int colourmaporigin;
	short int colourmaplength;
	char  colourmapdepth;
	short int x_origin;
	short int y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
} TargaHeader;
#pragma pack(pop)

typedef struct {
   unsigned char r,g,b,a;
} PIXEL;

class Targa
{
private:

	// Reader params
	DWORD	m_dCurrentPos;
	void	*m_pSource;
	DWORD	m_dSourceType;

	Targa();
	~Targa();

	int Read(void *data, int size, int count);
	int Seek(DWORD size, DWORD type);

public:
	TargaHeader m_Header;
	PIXEL		*m_pData;

	static Targa *LoadTargaImage(void *data, DWORD type);
	static Targa *LoadTargaImage(short int width, short int height, DWORD type, void *data);
	void Destroy();
	void *GetData(void *data, DWORD type);
	int SaveTargaImage(TCHAR *filename);
};

#endif