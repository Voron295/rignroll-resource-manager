#include "Targa.h"
#include <tchar.h>

WORD Get565FromRGB(BYTE r, BYTE g, BYTE b);
void GetRGBFrom565(WORD color, BYTE &r, BYTE &g, BYTE &b);
DWORD GetRGBAFrom4444(WORD color);
void GetRGBAFrom4444(WORD color, BYTE &r, BYTE &g, BYTE &b, BYTE &a);

Targa::Targa()
{
	memset(this, 0, sizeof(Targa));
}

Targa::~Targa()
{
	if(m_pData)
		delete[] m_pData;
	if(m_pSource && m_dSourceType == SOURCE_FILE)
		fclose((FILE*)m_pSource);
}

void MergeBytes(PIXEL *pixel,unsigned char *p,int bytes);

int Targa::Read(void *data, int size, int count)
{
	int result = 1;
	if(m_dSourceType == SOURCE_FILE)
	{
		result = fread(data, size, count, (FILE*)m_pSource);
	}
	else
	{
		for(int i = 0; i < count * size; i += size)
		{
			result = (int)memcpy(&((BYTE*)data)[m_dCurrentPos + i], m_pSource, size);
		}
	}
	m_dCurrentPos += count * size;
	return result;
}

int Targa::Seek(DWORD size, DWORD type)
{
	int result;
	if(type == SEEK_CUR)
	{
		m_dCurrentPos += size;
	}
	else if(type == SEEK_SET)
	{
		m_dCurrentPos = size;
	}
	else
		return 0;

	if(m_dSourceType == SOURCE_FILE)
		result = fseek((FILE*)m_pSource, m_dCurrentPos, SEEK_SET);
	else
		result = 1;
	return result;
}

Targa *Targa::LoadTargaImage(void *data, DWORD type)
{
	Targa *img = new Targa();
	if(!img)
		return 0;

	img->m_dSourceType = type;

	FILE *f;
	if(type == SOURCE_FILE)
	{
		f = _tfopen((TCHAR*)data, TEXT("rb"));

		if(!f)
		{
			img->~Targa();
			return 0;
		}
		img->m_pSource = f;
	}
	else if(type == SOURCE_MEMORY)
	{
		img->m_pSource = data;
	}
	else
	{
		img->~Targa();
		return 0;
	}

	img->Read(&img->m_Header.idlength, 1, 1);
	img->Read(&img->m_Header.colourmaptype, 1, 1);
	img->Read(&img->m_Header.datatypecode, 1, 1);

	img->Read(&img->m_Header.colourmaporigin, 2, 1);
	img->Read(&img->m_Header.colourmaplength, 2, 1);
	img->Read(&img->m_Header.colourmapdepth, 1, 1);

	img->Read(&img->m_Header.x_origin, 2, 1);
	img->Read(&img->m_Header.y_origin, 2, 1);
	img->Read(&img->m_Header.width, 2, 1);
	img->Read(&img->m_Header.height, 2, 1);
	img->Read(&img->m_Header.bitsperpixel, 1, 1);
	img->Read(&img->m_Header.imagedescriptor, 1, 1);

	/* What can we handle */
	if (img->m_Header.datatypecode != 2 && img->m_Header.datatypecode != 10)
	{
		MessageBox(0, "Can only handle image type 2 and 10", "Error!", MB_OK | MB_ICONERROR);
		img->~Targa();
		return 0;
	}
	if (img->m_Header.bitsperpixel != 16 && 
		img->m_Header.bitsperpixel != 24 && img->m_Header.bitsperpixel != 32)
	{
		MessageBox(0, "Can only handle pixel depths of 16, 24, and 32", "Error!", MB_OK | MB_ICONERROR);
		img->~Targa();
		return 0;
	}
	if (img->m_Header.colourmaptype != 0 && img->m_Header.colourmaptype != 1)
	{
		MessageBox(0, "Can only handle colour map types of 0 and 1", "Error!", MB_OK | MB_ICONERROR);
		img->~Targa();
		return 0;
	}

	img->m_pData = new PIXEL[img->m_Header.width * img->m_Header.height];
	memset(img->m_pData, 0, img->m_Header.width * img->m_Header.height * sizeof(PIXEL));
	int skipover = img->m_Header.idlength;
	skipover += img->m_Header.colourmaptype * img->m_Header.colourmaplength;
	img->Seek(skipover, SEEK_CUR);

	/* Read the image */
	int bytes2read = img->m_Header.bitsperpixel / 8;
	int n = 0;
	int i, j;
	BYTE p[5];
	while (n < img->m_Header.width * img->m_Header.height)
	{
		if (img->m_Header.datatypecode == 2) /* Uncompressed */
		{                     
			if (img->Read(p,1,bytes2read) != bytes2read)
			{
				MessageBox(0, "Unexpected end of file at pixel\n", "Error", MB_OK | MB_ICONERROR);
				img->~Targa();
				return 0;
			}
			MergeBytes(&(img->m_pData[n]),p,bytes2read);
			n++;
		}
		else if (img->m_Header.datatypecode == 10) /* Compressed */
		{             
			if (img->Read(p,1,bytes2read+1) != bytes2read+1)
			{
				MessageBox(0, "Unexpected end of file at pixel\n", "Error", MB_OK | MB_ICONERROR);
				img->~Targa();
				return 0;
			}
			j = p[0] & 0x7f;
			MergeBytes(&(img->m_pData[n]),&(p[1]),bytes2read);
			n++;
			if (p[0] & 0x80) /* RLE chunk */
			{         
				for (i=0;i<j;i++)
				{
					MergeBytes(&(img->m_pData[n]),&(p[1]),bytes2read);
					n++;
				}
			}
			else
			{                   /* Normal chunk */
				for (i=0;i<j;i++)
				{
					if (img->Read(p,1,bytes2read) != bytes2read)
					{
						MessageBox(0, "Unexpected end of file at pixel\n", "Error", MB_OK | MB_ICONERROR);
						img->~Targa();
						return 0;
					}
					MergeBytes(&(img->m_pData[n]),p,bytes2read);
					n++;
				}
			}
		}
	}

	if(type == SOURCE_FILE)
		fclose((FILE*)img->m_pSource);

	img->m_pSource = 0;
	img->m_dSourceType = 0;
	return img;
}

void MergeBytes(PIXEL *pixel,unsigned char *p,int bytes)
{
   if (bytes == 4) {
      pixel->r = p[2];
      pixel->g = p[1];
      pixel->b = p[0];
      pixel->a = p[3];
   } else if (bytes == 3) {
      pixel->r = p[2];
      pixel->g = p[1];
      pixel->b = p[0];
      pixel->a = 255;
   } else if (bytes == 2) {
      pixel->r = (p[1] & 0x7c) << 1;
      pixel->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
      pixel->b = (p[0] & 0x1f) << 3;
      pixel->a = (p[1] & 0x80);
   }
}

void *Targa::GetData(void *data, DWORD type)
{
	if(type == IMAGE_RGB_565)
	{
		for(int i = m_Header.height - 1; i >= 0; i--)
		{
			for(int j = 0; j < m_Header.width; j++)
			{
				int k = i * m_Header.width + j;
				((WORD*)data)[(m_Header.height - i - 1) * m_Header.width + j] = Get565FromRGB(m_pData[k].r, m_pData[k].g, m_pData[k].b);
			}
		}
	}
	else if(type == IMAGE_RGBA_4444)
	{
		for(int i = m_Header.height - 1; i >= 0; i--)
		{
			for(int j = 0; j < m_Header.width; j++)
			{
				int k = i * m_Header.width + j;
				((WORD*)data)[(m_Header.height - i - 1) * m_Header.width + j] = (((m_pData[k].a)/16) << 12) | ((m_pData[k].b/16) << 8) | ((m_pData[k].g/16) << 4) | (m_pData[k].r/16);
			}
		}
	}
	else if(type == IMAGE_RGB_888)
	{
		for(int i = m_Header.height - 1; i >= 0; i--)
		{
			for(int j = 0; j < m_Header.width; j++)
			{
				int k = i * m_Header.width + j;
				((BYTE*)data)[((m_Header.height - i - 1) * m_Header.width + j)*3] = m_pData[k].r;
				((BYTE*)data)[((m_Header.height - i - 1) * m_Header.width + j)*3+1] = m_pData[k].g;
				((BYTE*)data)[((m_Header.height - i - 1) * m_Header.width + j)*3+2] = m_pData[k].b;
			}
		}
	}
	else if(type == IMAGE_RGBA_8888)
	{
		for(int i = m_Header.height - 1; i >= 0; i--)
		{
			for(int j = 0; j < m_Header.width; j++)
			{
				int k = i * m_Header.width + j;
				((BYTE*)data)[((m_Header.height - i - 1) * m_Header.width + j)*4] = m_pData[k].r;
				((BYTE*)data)[((m_Header.height - i - 1) * m_Header.width + j)*4+1] = m_pData[k].g;
				((BYTE*)data)[((m_Header.height - i - 1) * m_Header.width + j)*4+2] = m_pData[k].b;
				((BYTE*)data)[((m_Header.height - i - 1) * m_Header.width + j)*4+3] = m_pData[k].a;
			}
		}
	}
	return data;
}

Targa *Targa::LoadTargaImage(short int width, short int height, DWORD type, void *data)
{
	Targa *img = new Targa();
	if(!img)
		return 0;

	img->m_Header.idlength = 0;
	img->m_Header.colourmaptype = 0;
	img->m_Header.datatypecode = 2;

	img->m_Header.colourmaporigin = 0;
	img->m_Header.colourmaplength = 0;
	img->m_Header.colourmapdepth = 0;

	img->m_Header.x_origin = 0;
	img->m_Header.y_origin = 0;
	img->m_Header.width = width;
	img->m_Header.height = height;

	img->m_Header.bitsperpixel = 32;
	if(type == IMAGE_RGBA_4444 || type == IMAGE_RGBA_8888)
		img->m_Header.imagedescriptor = 8;
	else
		img->m_Header.imagedescriptor = 0;

	img->m_pData = new PIXEL[width * height];
	memset(img->m_pData, 0, width * height * sizeof(PIXEL));

	if(type == IMAGE_RGB_565)
	{
		for(int i = img->m_Header.height - 1; i >= 0; i--)
		{
			for(int j = 0; j < img->m_Header.width; j++)
			{
				int k = i * img->m_Header.width + j;
				GetRGBFrom565(((WORD*)data)[(img->m_Header.height - i - 1) * img->m_Header.width + j], 
					img->m_pData[k].r, img->m_pData[k].g, img->m_pData[k].b);
			}
		}
	}
	else if(type == IMAGE_RGBA_4444)
	{
		for(int i = img->m_Header.height - 1; i >= 0; i--)
		{
			for(int j = 0; j < img->m_Header.width; j++)
			{
				int k = i * img->m_Header.width + j;
				GetRGBAFrom4444(((WORD*)data)[(img->m_Header.height - i - 1) * img->m_Header.width + j], 
					img->m_pData[k].r, img->m_pData[k].g, img->m_pData[k].b, img->m_pData[k].a);
			}
		}
	}
	return img;
}

int Targa::SaveTargaImage(TCHAR *filename)
{
	FILE *f = _tfopen(filename, TEXT("wb"));

	if(!f)
		return 0;

	fwrite(&m_Header.idlength, 1, 1, f);
	fwrite(&m_Header.colourmaptype, 1, 1, f);
	fwrite(&m_Header.datatypecode, 1, 1, f);

	fwrite(&m_Header.colourmaporigin, 2, 1, f);
	fwrite(&m_Header.colourmaplength, 2, 1, f);
	fwrite(&m_Header.colourmapdepth, 1, 1, f);

	fwrite(&m_Header.x_origin, 2, 1, f);
	fwrite(&m_Header.y_origin, 2, 1, f);
	fwrite(&m_Header.width, 2, 1, f);
	fwrite(&m_Header.height, 2, 1, f);
	fwrite(&m_Header.bitsperpixel, 1, 1, f);
	fwrite(&m_Header.imagedescriptor, 1, 1, f);

	for(int i = 0; i < m_Header.width * m_Header.height; i++)
	{
		fwrite(&m_pData[i], 1, 4, f);
	}
	fclose(f);
	return 1;
}

void Targa::Destroy()
{
	this->~Targa();
}