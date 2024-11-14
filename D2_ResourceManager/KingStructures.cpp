#include <windows.h>
#include <iostream>
#include <string>
#include "KingStructures.h"
#include <GL/glu.h>
//#include <gl/glui.h>
//#include <gl/glut.h>
//#include <GL/glext.h>
#include "CCamera.h"
//#include "CNode.h"
#include "UI_Elements.h"
#include "PathManager.h"
#include "Shlobj.h"
//#include "vld.h"
#include "FileManager.h"

extern WinTree *g_pTree;

#define GL_UNSIGNED_SHORT_4_4_4_4_REV 33637
#define GL_UNSIGNED_SHORT_5_6_5 33635


char fileName[512];
FILE *file;
FILE *logFile;
char tmp[512];
int lvl = 0;

//RGB565 -> RGB888 using tables
BYTE Table5[] = {0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132,
 140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255};
 
BYTE Table6[] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69,
 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138,
 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198,
 202, 206, 210, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255};

void GetRGBFrom565(WORD color, BYTE &r, BYTE &g, BYTE &b)
{
	b = (color & 0xF800) >> 11;
	g = (color & 0x7E0) >> 5;
	r = (color & 0x1F);
	/*
	r = r * 0xFF / 0x1F;
	g = g * 0xFF / 0x3F;
	b = b * 0xFF / 0x1F;*/

	r = Table5[r];
	g = Table6[g];
	b = Table5[b];
}

WORD Get565FromRGB(BYTE r, BYTE g, BYTE b)
{
	r = r * 0x1F / 0xFF;
	g = g * 0x3F / 0xFF;
	b = b * 0x1F / 0xFF;
	return (r << 11) | (g << 5) | b;
}

WORD Get565From555(WORD color)
{
	BYTE r, g, b;
	r = (color & 0x7C00) >> 10;
	g = (color & 0x3E0) >> 5;
	b = (color & 0x1F);
	b = b * 255 / 31;
	g = g * 255 / 31;
	r = r * 255 / 31;
	return Get565FromRGB(r, g, b);
}

DWORD GetRGBAFrom4444(WORD color)
{
	BYTE out[4];
	out[0] = ((color & 0xF000) >> 12) * 17;
	out[3] = ((color & 0xF00) >> 8) * 17;
	out[2] = ((color & 0xF0) >> 4) * 17;
	out[1] = (color & 0xF) * 17;
	return *(DWORD *)&out;
}

void GetRGBAFrom4444(WORD color, BYTE &r, BYTE &g, BYTE &b, BYTE &a)
{
	a = ((color & 0xF000) >> 12) * 17;
	r = ((color & 0xF00) >> 8) * 17;
	g = ((color & 0xF0) >> 4) * 17;
	b = (color & 0xF) * 17;
}

WORD FileReadWord(WORD value, FILE *file)
{
	fread(&value, 2, 1, file);
	return value;
}

DWORD FileReadInt(DWORD value, FILE *file)
{
	fread(&value, 4, 1, file);
	return value;
}

float FileReadFloat(float value, FILE *file)
{
	fread(&value, 4, 1, file);
	return value;
}

void FileWriteWord(WORD value, FILE *file)
{
	fwrite(&value, 2, 1, file);
}

void FileWriteInt(DWORD value, FILE *file)
{
	fwrite(&value, 4, 1, file);
}

void FileWriteFloat(float value, FILE *file)
{
	fwrite(&value, 4, 1, file);
}

void ObjWriteVertex(float x, float y, float z, FILE *file)
{
	fprintf(file, "v %f %f %f\n", x, y, z);
}

void ObjWriteFace(int a, int b, int c, FILE *file)
{
	fprintf(file, "f %d %d %d\n", a, b, c);
}

void ObjWriteFaceWithUV(int a, int b, int c, FILE *file)
{
	fprintf(file, "f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c);
}

void ObjWriteGroup(char *group, FILE *file)
{
	fprintf(file, "g %s\n", group);
}

void ObjWriteUV(float u, float v, FILE *file)
{
	fprintf(file, "vt %f %f\n", u, v);
}

int ResTexture::g_iGLTextureCount = 0;
void CALLBACK OnWinTreeTextureEvent(WinTree *tree, WinTreeItem *item, DWORD eventId, DWORD flags);

void CElement::ReadFaces(FILE *file, int count)
{
	Poly tmpPoly;
	Poly addPoly;
	//memset(&tmpPoly, 0, sizeof(tmpPoly));
	//memset(&addPoly, 0, sizeof(tmpPoly));

	int Buf;
	//int polyCount = count;
	//polys = new Poly[count];
	for(int i = 0; i < count; i++)
	{
		tmpPoly.vertices.clear();
		int type;
		fread(&type, 4, 1, file);
		tmpPoly.internalFormat = type;
		fread(&Buf, 4, 1, file);
		fread(&Buf, 4, 1, file);
		fread(&tmpPoly.materialId, 4, 1, file);

		fread(&tmpPoly.vertsCount, 4, 1, file);
		//tmpPoly.vertices = new int[tmpPoly.vertsCount];
		//fprintf(logFile, "\nvert_count: %d\n", vert_count);
		for (int j = 0; j < tmpPoly.vertsCount; j++)
		{
			int vertId = 0;
			switch (type)
			{
			case 2:
			case 3:
				{
					  fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file); // u
					fread(&Buf, 4, 1, file); // v
					break;
				}

			case 0x30:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file); // unk floats
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					break;
				}

			case 0x31:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file); // unk float
					break;
				}
			
			case 0x32:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file); // unk floats
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					break;
				}

			case 0x33:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file); // unk floats
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					break;
				}

			case 0x83:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					break;
				}

			case 0xB0:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					break;
				}
				
			case 0xB1:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file);
					break;
				}

			case 0xB2:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					break;
				}
			case 0xB3:
				{
						 fread(&vertId, 4, 1, file); // vert_id
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					break;
				}

			case 0x0:
			case 0x1:
			case 0x10:
			case 0x11:
			case 0x80:
			case 0x81:
			case 0x90:
			case 0x91:
				{
						 fread(&vertId, 4, 1, file);
					break;
				}
			default:
				{
					//fprintf(logFile, "UNKNOWN: %d At 0x%X\n", id_unk, ftell(file));
					   fread(&vertId, 4, 1, file);
					break;
				}
			}
			tmpPoly.vertices.push_back(vertId);
		}

		addPoly.internalFormat = type;
		addPoly.materialId = tmpPoly.materialId;

		if (tmpPoly.vertsCount > 3 && type != 0x10 && type != 0x0 && type != 0x1)
		{
			for (int k = 0; k < (tmpPoly.vertsCount - 2); k++)
			{
				addPoly.vertices.clear();
				if ((k % 2) != 0)
				{
					addPoly.vertices.push_back(tmpPoly.vertices[k + 2]);
					addPoly.vertices.push_back(tmpPoly.vertices[k + 1]);
					addPoly.vertices.push_back(tmpPoly.vertices[k]);
				}
				else
				{
					addPoly.vertices.push_back(tmpPoly.vertices[k]);
					addPoly.vertices.push_back(tmpPoly.vertices[k + 1]);
					addPoly.vertices.push_back(tmpPoly.vertices[k + 2]);
				}
				addPoly.vertsCount = addPoly.vertices.size();
				polys.push_back(addPoly);
			}
		}
		else
		{
			for (int k = 1; k < tmpPoly.vertsCount - 1; k++)
			{
				addPoly.vertices.clear();

				addPoly.vertices.push_back(tmpPoly.vertices[0]);
				addPoly.vertices.push_back(tmpPoly.vertices[k]);
				addPoly.vertices.push_back(tmpPoly.vertices[k + 1]);

				addPoly.vertsCount = addPoly.vertices.size();
				polys.push_back(addPoly);
			}
		}
	}
}

CModel *currentModel = 0;
int elementsCount = 0;
int currentElement = 0;

CBlock *CBlock::Read(FILE *file, WinTreeItem *item, B3D *b3d)
{
	lvl++;
	
	CBlock *block = new CBlock();
	if(!block)
		return 0;

	block->m_fX = 0;
	block->m_fY = 0;
	block->m_fZ = 0;
	block->m_fAngle = 0;

	block->m_pB3D = b3d;

	int Buf = 0;
	char textBuf[64];
	while(Buf != 333)
	{
		if(!fread(&Buf, 4, 1, file))
			return 0;
		if(Buf == 222)
			return 0;
	}
	int blockPos = ftell(file);
	fread(block->m_sName, 1, 32, file); // Name
	fread(&block->m_dType, 4, 1, file);
	if(strlen(block->m_sName))
	{
		sprintf(tmp, "%s (0x%X)(0x%X)", block->m_sName, block->m_dType, blockPos);
	}
	else
	{
		sprintf(block->m_sName, "Noname_0x%X", blockPos);
		sprintf(tmp, "Noname (0x%X)(0x%X)", block->m_dType, blockPos);
	}

	WinTreeItem *blockItem = 0;
	if(item)
	{
		blockItem = item->AddChild(tmp, 0);

		blockItem->SetCallback(CModel::OnWinTreeEvent);
		blockItem->SetData((DWORD)block);
	}
	switch(block->m_dType)
	{
	case 0:
		{
			int count;

			for(int i = 0; i < 10; i++)
				fread(&Buf, 4, 1, file);
			fread(&count, 4, 1, file);
			
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}
	
	case 0x1:
		{
			fread(&textBuf, 1, 32, file); // name2
			if(blockItem)
				blockItem->AddChild(textBuf, 0);
			//fprintf(logFile, "name1: %s\n", textBuf);
			fread(&textBuf, 1, 32, file); // name2
			//fprintf(logFile, "name2: %s\n", textBuf);
			break;
		}

	case 0x2:
		{
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file); // blocks count
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x3:
		{
			int count;

			for (int i = 0; i < 4; i++)
				fread(&Buf, 4, 1, file);
			fread(&count, 4, 1, file);

			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for (int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if (*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x4:
		{
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&textBuf, 1, 32, file);
			fread(&textBuf, 1, 32, file);
			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x5:	// CShowBlock
		{		// Блок, содержащий в себе другие блоки
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&textBuf, 1, 32, file); // name2
			fread(&count, 4, 1, file); // blocks count
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x7:
		{
			CModel *Mblock = CModel::Read(file, blockItem, 7, b3d);
			strcpy(Mblock->m_sName, block->m_sName);
			block->~CBlock();
			block = Mblock;
			/*int count;
			for(int i = 0; i < 4; i++) // unk int & 3 float
				fread(&Buf, 4, 1, file);

			fread(&textBuf, 1, 32, file); // name2
			fread(&count, 4, 1, file);

			for(int i = 0; i < count; i++)
			{
				fread(&Buf, 4, 1, file); // pos & uv
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
			}
			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}*/
			break;
		}

	case 0x8: // Информация о полигонах (материалы, номера вертексов и т.п.)
		{
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&count, 4, 1, file);

			// CFace, куча непонятной херни
			if(currentModel)
			{
				currentModel->m_pElements[currentElement++].ReadFaces(file, count);
			}
			else
			{
				CElement *elem = new CElement();
				elem->ReadFaces(file, count);
				elem->~CElement();
				elementsCount++;
			}
			block->~CBlock();
			break;
		}

	case 0x9: // Типа хедер к коллизии
		{
			int count;
			for(int i = 0; i < 4; i++)
				fread(&Buf, 4, 1, file);

			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0xA:	// Блок, содержащий половину нормальных моделей и половину лодов
		{		// Хотя внутри попадаются типы моделей (0x25)
			int count;
			for(int i = 0; i < 4; i++) // unk int & 3 floats
				fread(&Buf, 4, 1, file);

			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&count, 4, 1, file); // blocks count
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < (count/2); i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			WinTreeItem *lods = 0;
			if(blockItem)
				lods = blockItem->AddChild("LOD", 0, CModel::OnWinTreeEvent);
			for(int i = count/2; i < count; i++)
			{
				*next = Read(file, lods, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0xC:
		{
			for(int i = 0; i < 11; i++)
				fread(&Buf, 4, 1, file);
			break;
		}

	case 0xD:	// хз пока что, обычно внутри блока 0xA
		{		// как-то связано с эвентом ежей
			int count;
			for(int i = 0; i < 4; i++) // unk int & 3 floats
				fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&count, 4, 1, file);
			fread(&Buf, 4, 1, file);

			//for(int i = 0; i < count; i++)
				//ReadBlock(file);
			break;
		}

	case 0xE:	// Какой-то евент
		{
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
		}

	case 0x12:	// какой-то "anmobj000xxxx"
		{
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&textBuf, 1, 32, file); // groupname1
			if(strlen(textBuf) && blockItem)
				blockItem->AddChild(textBuf, 0);//fprintf(logFile, "	Subname: %s\n", textBuf);
			fread(&textBuf, 1, 32, file); // groupname2
			//if(strlen(textBuf))
				//fprintf(logFile, "	Groupname: %s\n", textBuf);
			break;
		}

	case 0x13: // CRoom короч
		{
			int count;
			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x14: // По ходу какая-то коллизия
		{
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);
			fread(&count, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			for(int i = 0; i < count; i++)
			{
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
			}
			//int curPos = ftell(file);
			break;
		}

	case 0x15: // Key block
		{
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&Buf, 4, 1, file); // integers
			fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x17: // Коллизия
		{
			int count;
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&count, 4, 1, file);
			for(int i = 0; i < count; i++)
			{
				fread(&Buf, 4, 1, file);
			}
			fread(&count, 4, 1, file);

			for(int i = 0; i < count; i++)
			{
				int vert_count;
				fread(&vert_count, 4, 1, file);
				for(int j = 0; j < vert_count; j++)
				{
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
				}
			}
			break;
		}

	case 0x18:	// Описание некого пространства CBlockSpace
		{
			int count;

			for(int i = 0; i < 9; i++)	// maybe matrix 3x3?
				fread(&Buf, 4, 1, file);

			for(int i = 0; i < 4; i++)
				fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x19:
		{
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&textBuf, 1, 32, file);

			// pos
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			fread(&textBuf, 1, 32, file); // короч 4 флоата каких-то
			break;
		}

	case 0x1C:
		{
			int count;
			for(int i = 0; i < 7; i++)
				fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file);

			for(int i = 0; i < count; i++)
			{
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
			}
			break;
		}

	case 0x1E:
		{
			// pos & rot
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&textBuf, 1, 32, file); // name

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			break;
		}

	case 0x1D:
		{
			int count;
			// pos & rot
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&count, 4, 1, file);

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			for(int i = 0; i < count; i++)
				fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}
			break;
		}

	case 0x21:
		{
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file);
			for(int i = 0; i < count; i++)
			{
				fread(&Buf, 4, 1, file); // floats
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);

				fread(&Buf, 4, 1, file); // some id
			}
			break;
		}

	case 0x22: // возможно, коллизия
		{
			int count;
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file);

			for(int i = 0; i < count; i++)
			{
				fread(&Buf, 4, 1, file); // floats
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
			}
			break;
		}

	case 0x23:	// Инфа о полигонах
		{
			int count;
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&count, 4, 1, file);

			if(currentModel)
			{
				currentModel->m_pElements[currentElement++].ReadFaces(file, count);
			}
			else
			{
				CElement *elem = new CElement();
				elem->ReadFaces(file, count);
				elem->~CElement();
				elementsCount++;
			}
			block->~CBlock();
			break;
		}

	case 0x24:
		{
			CModel *Mblock = CModel::Read(file, blockItem, 0x24, b3d);
			strcpy(Mblock->m_sName, block->m_sName);
			block->~CBlock();
			block = Mblock;
			/*int count, count2;
			for(int i = 0; i < 4; i++)	// idk wtf
				fread(&Buf, 4, 1, file); // maybe pos & angle

			fread(&textBuf, 1, 64, file); // name2
			if(blockItem)
				blockItem->AddChild(textBuf, 0);
			//fprintf(logFile, "subName: %s\n", textBuf);
			fread(&count, 4, 1, file);
			fread(&count2, 4, 1, file);
			//fprintf(logFile, "counts: %d %d\n", count, count2);
			for(int i = 0; i < count; i++)
			{
				for(int j = 0; j < count2; j++)
				{
					fread(&Buf, 4, 1, file); // some floats
					fread(&Buf, 4, 1, file); // some floats
					fread(&Buf, 4, 1, file); // some floats
					fread(&Buf, 4, 1, file); // some floats
					//fprintf(logFile, "float(%d, %d): %f\n", i, j, *(float *)&Buf);
				}
			}

			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}*/
			break;
		}

	case 0x25:	// Блок, содержащий информацию о вертексах в модели
		{		// Также содержит другие блоки, с инфой о полигонах из этих вертексов
					CModel *Mblock = CModel::Read(file, blockItem, 0x25, b3d);
					strcpy(Mblock->m_sName, block->m_sName);
					block->~CBlock();
					block = Mblock;
			/*int count, subType;
			for(int i = 0; i < 4; i++)	// pos & angle
				fread(&Buf, 4, 1, file);

			fread(&textBuf, 1, 32, file); // name2
			if(strlen(textBuf) && blockItem)
			{
				sprintf(tmp, "Name2: %s", textBuf);
				blockItem->AddChild(tmp, 0);
			}
			else if(blockItem)
			{
				blockItem->AddChild("Name2: Unnamed", 0);
			}
			fread(&subType, 4, 1, file);
			fread(&count, 4, 1, file);
			for(int i = 0; i < count; i++)
			{
				if(subType == 514)
				{
					for(int i = 0; i < 3; i++)	// pos
						fread(&Buf, 4, 1, file);

					for(int i = 0; i < 7; i++)	// unk floats
						fread(&Buf, 4, 1, file);

					fread(&Buf, 4, 1, file); // uv
					fread(&Buf, 4, 1, file);
				}
				else
				{
					for(int i = 0; i < 3; i++)	// pos
						fread(&Buf, 4, 1, file);

					if(subType != 3) // uv
					{
						fread(&Buf, 4, 1, file);
						fread(&Buf, 4, 1, file);
					}

					if(subType == 258)
					{
						fread(&Buf, 4, 1, file); // 2 unk floats
						fread(&Buf, 4, 1, file);
					}

					if(subType == 515)
					{
						fread(&Buf, 4, 1, file);
						fread(&Buf, 4, 1, file);
						fread(&Buf, 4, 1, file);
						fread(&Buf, 4, 1, file);
						fread(&Buf, 4, 1, file);
					}
					else
					{
						for(int i = 0; i < 3; i++)	// normal
							fread(&Buf, 4, 1, file);
					}
				}
			}

			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}*/
			break;
		}

	case 0x27:
		{
			int count;
			// floats
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);

			fread(&Buf, 4, 1, file); // int

			fread(&count, 4, 1, file);
			block->m_pChildCount = count;
			CBlock **next = &block->m_pChild;
			for(int i = 0; i < count; i++)
			{
				*next = Read(file, blockItem, b3d);
				if(*next)
					next = &((*next)->m_pNext);
			}

			break;
		}

	case 0x28: // Видимо генератор деревьев
		{
			int count;
			// pos & rot
			fread(&block->m_fX, 4, 1, file);
			fread(&block->m_fY, 4, 1, file);
			fread(&block->m_fZ, 4, 1, file);
			fread(&block->m_fAngle, 4, 1, file);

			fread(&textBuf, 1, 32, file); // pool name
			fread(&textBuf, 1, 32, file); // generator callback name

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&count, 4, 1, file);
			for(int i = 0; i < count; i++)
				fread(&Buf, 4, 1, file);
			break;
		}

	default:
		//if(!show) fprintf(logFile, "Unknown type: %d at 0x%X\n", type, blockPos);
		//else fprintf(logFile, "!unknown type!\n");
		sprintf(tmp, "Unknown type: %d at 0x%X\n", block->m_dType, blockPos);
		if(blockItem)
			blockItem->AddChild(tmp, 0);
		MessageBox(0, tmp, "Warning!", 0);
		return false;
	}

	lvl--;
	return block;
}

bool CBlock::WriteTo3DS(FILE *f)
{
	CBlock *block = this;
	while (block)
	{
		if (block->m_bIsModel)
		{
			block->WriteTo3DS(f);
		}
		else if (block->m_pChild)
		{
			block->m_pChild->WriteTo3DS(f);
		}
		block = block->m_pNext;
	}
	return true;
}

char lastContainer[32];
bool CBlock::WriteToObj(FILE **f, bool first)
{
	if (m_dType == 0x5)
		strcpy(lastContainer, m_sName);
	CBlock *block = m_pChild;
	while (block)
	{
		block->WriteToObj(f);
		/*
		if (block->m_bIsModel)
		{
			block->WriteToObj(f);
		}
		else if (block->m_pChild)
		{
			if (block->m_dType == 0x5)
				strcpy(lastContainer, block->m_sName);
			block->m_pChild->WriteToObj(f);
		}*/
		block = block->m_pNext;
	}
	if (first)
	{
		block = m_pNext;
		while (block)
		{
			block->WriteToObj(f);
			block = block->m_pNext;
		}
	}
	return true;
}

bool CBlock::WriteToFBX(FILE *f)
{
	CBlock *block = this;
	while (block)
	{
		if (block->m_bIsModel)
		{
			block->WriteToFBX(f);
		}
		else if (block->m_pChild)
		{
			block->m_pChild->WriteToFBX(f);
		}
		block = block->m_pNext;
	}
	return true;
}

CModel *CModel::Read(FILE *file, WinTreeItem *item, DWORD type, B3D *b3d)
{
	int count, subType;
	int Buf = 0;
	char textBuf[64];

	CModel *model = new CModel();
	if(!model)
		return 0;

	model->m_pB3D = b3d;

	fread(&model->m_fX, 4, 1, file);
	fread(&model->m_fY, 4, 1, file);
	fread(&model->m_fZ, 4, 1, file);
	fread(&model->m_fAngle, 4, 1, file);
	model->m_dType = type;

	item->SetCallback(CModel::OnWinTreeEvent);
	item->SetData((DWORD)model);

	fread(&textBuf, 1, 32, file); // name2
	if(type == 0x24)
		fread(&textBuf, 1, 32, file); // name3

	if(type == 0x25 || type == 0x24)
		fread(&subType, 4, 1, file);	// someSubType???
	model->m_dSubType = subType;
	fread(&model->m_iVerticesCount, 4, 1, file);		// count of vertices
	model->m_pVertices = new Vertex[model->m_iVerticesCount];
	//model->m_pVerticesPos = new CVector[model->m_iVerticesCount];
	//model->m_pUVs = new UV[model->m_iVerticesCount];
	for(int i = 0; i < model->m_iVerticesCount; i++)
	{
		model->m_pVertices[i].x = 0;
		model->m_pVertices[i].y = 0;
		model->m_pVertices[i].z = 0;
		model->m_pVertices[i].u = 0;
		model->m_pVertices[i].v = 0;
		/*model->m_pVerticesPos[i].x = 0;
		model->m_pVerticesPos[i].y = 0;
		model->m_pVerticesPos[i].z = 0;
		model->m_pUVs[i].u = 0;
		model->m_pUVs[i].v = 0;*/


		if(type != 0x25)
		{
			fread(&model->m_pVertices[i].x, 4, 1, file);
			fread(&model->m_pVertices[i].y, 4, 1, file);
			fread(&model->m_pVertices[i].z, 4, 1, file);

			fread(&model->m_pVertices[i].u, 4, 1, file); // uv
			fread(&model->m_pVertices[i].v, 4, 1, file);

			if(type == 0x24)
			{
				if(subType >> 8)
				{
					fread(&Buf, 4, 1, file);
					fread(&Buf, 4, 1, file);
				}

				if(subType == 1 || subType == 2)
				{
					fread(&Buf, 4, 1, file); // some vector
					fread(&Buf, 4, 1, file); // возможно вектор нормали
					fread(&Buf, 4, 1, file);
				}
				else if(subType == 3)
				{
					fread(&Buf, 4, 1, file);
				}
			}
			/*fread(&model->m_pVerticesPos[i].x, 4, 1, file);
			fread(&model->m_pVerticesPos[i].y, 4, 1, file);
			fread(&model->m_pVerticesPos[i].z, 4, 1, file);

			fread(&model->m_pUVs[i].u, 4, 1, file); // uv
			fread(&model->m_pUVs[i].v, 4, 1, file);*/
		}
		else if(subType == 514)
		{
			//for(int i = 0; i < 3; i++)	// pos
			fread(&model->m_pVertices[i].x, 4, 1, file);
			fread(&model->m_pVertices[i].y, 4, 1, file);
			fread(&model->m_pVertices[i].z, 4, 1, file);
			/*fread(&model->m_pVerticesPos[i].x, 4, 1, file);
			fread(&model->m_pVerticesPos[i].y, 4, 1, file);
			fread(&model->m_pVerticesPos[i].z, 4, 1, file);*/

			for(int j = 0; j < 7; j++)	// unk floats
				fread(&Buf, 4, 1, file);

			fread(&model->m_pVertices[i].u, 4, 1, file); // uv
			fread(&model->m_pVertices[i].v, 4, 1, file);
			/*fread(&model->m_pUVs[i].u, 4, 1, file); // uv
			fread(&model->m_pUVs[i].v, 4, 1, file);*/
		}
		else
		{
			//for(int i = 0; i < 3; i++)	// pos
			fread(&model->m_pVertices[i].x, 4, 1, file);
			fread(&model->m_pVertices[i].y, 4, 1, file);
			fread(&model->m_pVertices[i].z, 4, 1, file);
			/*fread(&model->m_pVerticesPos[i].x, 4, 1, file);
			fread(&model->m_pVerticesPos[i].y, 4, 1, file);
			fread(&model->m_pVerticesPos[i].z, 4, 1, file);*/

			if(subType != 3) // uv
			{
				fread(&model->m_pVertices[i].u, 4, 1, file); // uv
				fread(&model->m_pVertices[i].v, 4, 1, file);
				/*fread(&model->m_pUVs[i].u, 4, 1, file); // uv
				fread(&model->m_pUVs[i].v, 4, 1, file);*/
			}

			if(subType == 258)
			{
				fread(&Buf, 4, 1, file); // 2 unk floats
				fread(&Buf, 4, 1, file);
			}

			if(subType == 515)
			{
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
				fread(&Buf, 4, 1, file);
			}
			else
			{
				for(int i = 0; i < 3; i++)	// normal
					fread(&Buf, 4, 1, file);
			}
		}
	}

	// Now reading elements

	fread(&count, 4, 1, file);
	model->m_pChildCount = 0;
	
	
	elementsCount = 0;
	int curPos = ftell(file);
	// going to undestand how much elements there are
	for(int i = 0; i < count; i++)
	{
		CBlock::Read(file, 0, b3d)->~CBlock();
		//model->m_pElements[i].Read(file);
	}
	fseek(file, curPos, SEEK_SET);
	model->m_iElementCount = elementsCount;
	model->m_pElements = new CElement[model->m_iElementCount];

	currentModel = model;
	currentElement = 0;
	for(int i = 0; i < count; i++)
	{
		CBlock::Read(file, item, b3d)->~CBlock();
		//model->m_pElements[i].Read(file);
	}
	currentModel = 0;

	model->GenerateGlModel();
	return model;
}

bool CModel::WriteTo3DS(FILE *f)
{
	int block4110Len = m_iVerticesCount * 3 * 4 + 8;
	int block4120Len = 8;

	int faceCount = 0;
	for (int i = 0; i < m_iElementCount; i++)
	{
		for (int j = 0; j < m_pElements->polys.size(); j++)
			block4120Len += 2 * 4; // WORD * (3 verts + flag)

		faceCount += m_pElements->polys.size();
	}
	int block4100Len = 6 + block4120Len + block4110Len;


	// 0x4000
	int chunkPos = ftell(f);
	int chunkId = 0x4000;
	int len = 6 + block4100Len + strlen(m_sName) + 1;
	fwrite(&chunkId, 2, 1, f);
	fwrite(&len, 4, 1, f);
	fwrite(m_sName, 1, strlen(m_sName) + 1, f);

	//0x4100
	chunkId = 0x4100;
	fwrite(&chunkId, 2, 1, f);
	fwrite(&block4100Len, 4, 1, f);

	//0x4110
	chunkId = 0x4110;
	len = block4110Len;
	fwrite(&chunkId, 2, 1, f);
	fwrite(&len, 4, 1, f);
	fwrite(&m_iVerticesCount, 2, 1, f);

	for (int i = 0; i < m_iVerticesCount; i++)
	{
		fwrite(&m_pVertices->x, 4, 1, f);
		fwrite(&m_pVertices->y, 4, 1, f);
		fwrite(&m_pVertices->z, 4, 1, f);
	}

	//0x4120
	chunkId = 0x4120;
	len = block4120Len; // число граней
	fwrite(&chunkId, 2, 1, f);
	fwrite(&len, 4, 1, f);
	fwrite(&faceCount, 2, 1, f);

	for (int i = 0; i < m_iElementCount; i++)
	{
		for (int j = 0; j < m_pElements[i].polys.size(); j++)
		{
			FileWriteWord(m_pElements[i].polys[j].vertices[0], f);
			FileWriteWord(m_pElements[i].polys[j].vertices[1], f);
			FileWriteWord(m_pElements[i].polys[j].vertices[2], f);
			FileWriteWord(6, f);
		}
	}
	return true;
}

int exportVertCount = 0;
int fileCount = 1;

bool CModel::WriteToObj(FILE **file, bool first)
{
	if ((exportVertCount + m_iVerticesCount) > 30000)
	{
		fclose(*file);
		char filename[512];
		sprintf(filename, "%s%d.obj", m_pB3D->m_sName, fileCount++);
		*file = fopen(filename, "w");
		fprintf(*file, "mtllib %s.mtl\n", m_pB3D->m_sName);
		exportVertCount = 0;
	}

	FILE *f = *file;

	int lastMtl = -1;
	if (strncmp(m_sName, "Noname", 6) == 0)
		ObjWriteGroup(lastContainer, f);
	else
		ObjWriteGroup(m_sName, f);
	exportVertCount += m_iVerticesCount;
	for (int i = 0; i < m_iVerticesCount; i++)
	{
		ObjWriteVertex(m_pVertices[i].x, m_pVertices[i].y, m_pVertices[i].z, f);
		ObjWriteUV(m_pVertices[i].u, m_pVertices[i].v, f);
	}

	for (int i = 0; i < m_iElementCount; i++)
	{
		for (int j = 0; j < m_pElements[i].polys.size(); j++)
		{
			if (lastMtl != m_pElements[i].polys[j].materialId)
			{
				fprintf(f, "usemtl %s\n", m_pB3D->m_sMaterials[m_pElements[i].polys[j].materialId]);
				lastMtl = m_pElements[i].polys[j].materialId;
			}
			ObjWriteFaceWithUV(m_pElements[i].polys[j].vertices[0] - m_iVerticesCount,
				m_pElements[i].polys[j].vertices[1] - m_iVerticesCount, 
				m_pElements[i].polys[j].vertices[2] - m_iVerticesCount, f);
		}
	}

	return true;
}

bool CModel::WriteToFBX(FILE *file)
{
	elementsCount++;
	FILE *f = file;
	size_t startBlock = ftell(f);
	fWriteDword(0, f); // next block
	fWriteDword(2, f); // numProp
	//if (strncmp(m_sName, "Noname", 6) == 0)
	//	fWriteDword(14 + strlen(lastContainer), f);
	//else
		fWriteDword(14 + strlen(m_sName), f);
	fWriteByte(5, f);
	fputs("Model", f);
	fWriteByte('S', f);
	//if (strncmp(m_sName, "Noname", 6) == 0)
	//{
	//	fWriteDword(strlen(lastContainer), f);
	//	fputs(lastContainer, f);
	//}
	//else
	//{
		fWriteDword(strlen(m_sName), f);
		fputs(m_sName, f);
	//}
	fWriteByte('S', f);
	fWriteDword(4, f);
	fputs("Mesh", f);

	// Vertices
	size_t pos = ftell(f);
	fWriteDword(pos + m_iVerticesCount * 15 + 21, f); // next block
	fWriteDword(m_iVerticesCount * 3, f); // numProp
	fWriteDword(m_iVerticesCount * 15, f); // sizeProp
	fWriteByte(8, f);
	fputs("Vertices", f);

	for (int i = 0; i < m_iVerticesCount; i++)
	{
		fWriteByte('F', f);
		fWriteFloat(m_pVertices[i].x, f);
		fWriteByte('F', f);
		fWriteFloat(m_pVertices[i].y, f);
		fWriteByte('F', f);
		fWriteFloat(m_pVertices[i].z, f);
	}
	
	DWORD blockSize = 0;
	for (int i = 0; i < m_iElementCount; i++)
	{
		for (auto it = m_pElements[i].polys.begin(); it != m_pElements[i].polys.end(); it++)
		{
			blockSize += it->vertsCount;
		}
	}
		
	pos = ftell(f);
	fWriteDword(pos + blockSize * 5 + 31 , f); // next block
	fWriteDword(blockSize, f); // numProp
	fWriteDword(blockSize * 5, f); // sizeProp
	fWriteByte(18, f);
	fputs("PolygonVertexIndex", f);

	for (int i = 0; i < m_iElementCount; i++)
	{
		for (auto it = m_pElements[i].polys.begin(); it != m_pElements[i].polys.end(); it++)
		{
			for (int j = 0; j < it->vertsCount; j++)
			{
				fWriteByte('I', f);
				if (j == it->vertsCount - 1)
					fWriteDword(it->vertices[j] ^ -1, f);
				else
					fWriteDword(it->vertices[j], f);
			}
		}
	}

	for (int i = 0; i < 13; i++)
	{
		fWriteByte(0, f);
	}

	size_t endBlock = ftell(f);
	fseek(f, startBlock, SEEK_SET);
	fWriteDword(endBlock, f);
	fseek(f, endBlock, SEEK_SET);
	return true;
}

void CModel::GenerateGlModel()
{
	m_pGlList = glGenLists(1);
	glNewList(m_pGlList,GL_COMPILE);
	for(int i = 0; i < m_iElementCount; i++)
	{
		for(int j = 0; j < m_pElements[i].polys.size(); j++)
		{
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glBindTexture(GL_TEXTURE_2D, 0);
			if(m_pElements[i].polys[j].materialId >= 0 && m_pB3D && m_pB3D->m_pRes
				&& m_pElements[i].polys[j].materialId < m_pB3D->m_iMaterialsCount)
			{
				ResTexture *texture = m_pB3D->m_pRes->GetByName(m_pB3D->m_sMaterials[m_pElements[i].polys[j].materialId]);
				if(texture)
					texture->glSetActive();
			}
			glBegin(GL_TRIANGLES);
			for (int k = 0; k < m_pElements[i].polys[j].vertices.size(); k++)
			{
				int id = m_pElements[i].polys[j].vertices[k];
				glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
				glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);
			}
			glEnd();
			glCullFace(GL_FRONT);
			glDisable(GL_CULL_FACE);
			/*DWORD type = m_pElements[i].polys[j].internalFormat;
			if(m_pElements[i].polys[j].vertsCount > 3 && type != 0x10 && type != 0x0 && type != 0x1)
			{
				glBegin(GL_TRIANGLES);
				for(int k = 0; k < (m_pElements[i].polys[j].vertsCount - 2); k++)
				{
					if((i % 2) == 0)
					{
						int id = m_pElements[i].polys[j].vertices[k+2];
						glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
						glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);

						id = m_pElements[i].polys[j].vertices[k+1];
						glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
						glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);

						id = m_pElements[i].polys[j].vertices[k];
						glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
						glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);
					}
					else
					{
						int id = m_pElements[i].polys[j].vertices[k];
						glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
						glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);

						id = m_pElements[i].polys[j].vertices[k+1];
						glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
						glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);

						id = m_pElements[i].polys[j].vertices[k+2];
						glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
						glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);
					}
				}
				glEnd();
			}
			else
			{
				glBegin(GL_POLYGON);
				for(int k = 0; k < m_pElements[i].polys[j].vertsCount; k++)
				{
					int id = m_pElements[i].polys[j].vertices[k];
					glTexCoord2f(m_pVertices[id].u, m_pVertices[id].v);
					glVertex3f(m_pVertices[id].x, m_pVertices[id].y, m_pVertices[id].z);
				}
				glEnd();
			}*/
		}
	}

	glEndList();
}

void CModel::glRender()
{
	glCallList(m_pGlList);
}

CElement *CElement::Read(FILE *file)
{
	int Buf = 0;
	int type;
	char textBuf[128];

	while(Buf != 333)
	{
		fread(&Buf, 4, 1, file);
		if(Buf == 222)
			return 0;
	}
	//int blockPos = ftell(file);
	fread(textBuf, 1, 32, file); // Name
	fread(&type, 4, 1, file);

	switch(type)
	{
	case 0x8:
		{
			int count;
			for(int i = 0; i < 4; i++) // pos & angle
				fread(&Buf, 4, 1, file);

			fread(&count, 4, 1, file);

			// CFace, куча непонятной херни
			ReadFaces(file, count);
			break;
		}

	case 0x23:
		{
			int count;
			for(int i = 0; i < 4; i++) // pos & angle
				fread(&Buf, 4, 1, file);

			fread(&Buf, 4, 1, file);
			fread(&Buf, 4, 1, file);
			fread(&count, 4, 1, file);
			ReadFaces(file, count);
			break;
		}
	}
	return this;
}

char *GetFileName(char *path, char *buf)
{
	for(int i = strlen(path) - 1; i > 0; i--)
	{
		if(path[i] == '.')
		{
			strncpy(buf, path, i);
			buf[i] = 0;
			break;
		}
	}
	return buf;
}

B3D *B3D::Load(char *fileName, WinTreeItem *item)
{
	int Buf;
	char textBuf[512];

	char pathToRes[512];

	// Init new object
	B3D *b3d = new B3D();
	if(!b3d)
		return 0;
	
	item->SetData((DWORD)b3d, 2);
	TreeView_SetCheckState(item->m_pTree->m_hWnd, item->m_hItem, 1);

	GetFileName(fileName, pathToRes);
	strcpy(b3d->m_sName, pathToRes);
	strcat(pathToRes, ".res");

	b3d->m_pRes = RES::Load(pathToRes);

	int count = 0;
	file = fopen(fileName, "rb");
	if(!file)
	{
		MessageBox(0, "Trouble with filename!", "Resource Manager", MB_OK);//cout << "Trouble with filename!";
		return 0;
	}

	// Main code goes here
	// 1. Reading header
	fread(&textBuf, 1, 4, file); // b3d
	fread(&Buf, 4, 1, file); // Unk offset
	fread(&Buf, 4, 1, file); // Unk count
	fread(&Buf, 4, 1, file); // Unk count
	int blocks_count;
	fread(&blocks_count, 4, 1, file); // Unk count
	fread(&Buf, 4, 1, file); // Unk offset
	fread(&b3d->m_iMaterialsCount, 4, 1, file); // Material count
	b3d->m_sMaterials = new (char *[b3d->m_iMaterialsCount]);
	WinTreeItem *matTree = 0;
	if(item)
		matTree = item->AddChild("Materials", 0);
	//fprintf(logFile, "Material count: %d\n", mat_count);

	// 2. Reading materials
	for(int i = 0; i < b3d->m_iMaterialsCount; i++)
	{
		char *matName = new char[32];
		b3d->m_sMaterials[i] = matName;
		fread(matName, 1, 32, file); // Name
		if(matTree)
			matTree->AddChild(matName, 0);

		//fprintf(logFile, "%s\n", textBuf);
	}

	// 3. Reading blocks
	WinTreeItem *blocksItem = 0;
	if(item)
	{
		blocksItem = matTree->AddNext("Blocks", 0);
		blocksItem->SetCallback(CModel::OnWinTreeEvent);
		blocksItem->SetData(0);
	}

	CBlock **nextBlock = &b3d->m_pBlocks;
	while(true)
	{
		*nextBlock = CBlock::Read(file, blocksItem, b3d);
		if(*nextBlock)
			nextBlock = &((*nextBlock)->m_pNext);
		else
			break;
	}
	fclose(file);
	//printf("finished\n");
	//getchar();
	//getchar();

	return b3d;
}

bool B3D::SaveTo3DS()
{
	char filename[512];
	strcpy(filename, m_sName);
	strcat(filename, ".3ds");

	FILE *f = fopen(filename, "wb");
	if (!f)
		return false;

	unsigned short chunkId = 0x4D4D; // main chunk id
	unsigned int len = 0;
	int ver = 3;
	fwrite(&chunkId, 2, 1, f);
	fwrite(&len, 4, 1, f);

	// Блок 0x0002
	chunkId = 0x0002;
	len = 10;
	fwrite(&chunkId, 2, 1, f);
	fwrite(&len, 4, 1, f);
	fwrite(&ver, 4, 1, f);

	m_pBlocks->WriteTo3DS(f);

	len = ftell(f);
	fseek(f, 2, SEEK_SET);
	fwrite(&len, 4, 1, f);
	fclose(f);
}

bool B3D::SaveToObj(const char *path)
{
	std::string sPath = path;
	if (m_pRes)
		m_pRes->SaveToMtl(sPath.substr(0, sPath.find_last_of(".")).c_str());

	exportVertCount = 0;
	fileCount = 1;

	FILE *f = fopen(path, "w");
	if (!f)
		return false;

	fprintf(f, "mtllib %s.mtl\n", m_sName);
	m_pBlocks->WriteToObj(&f, true);

	fclose(f);
}

void WriteConnectionsFBX(FILE *f, CBlock *block)
{
	while (block)
	{
		if (block->m_bIsModel)
		{
			size_t startBlock = ftell(f);
			fWriteDword(startBlock + strlen(block->m_sName) + 49, f); // endOffset
			fWriteDword(3, f); // numProps
			fWriteDword(29 + strlen(block->m_sName), f); // propsLen
			fWriteByte(7, f); // nameLen
			fputs("Connect", f);

			// 1
			fWriteByte('S', f);	// 0
			fWriteDword(2, f);	// 1
			fputs("OO", f);		// 5

			// 2
			fWriteByte('S', f);	// 7
			fWriteDword(strlen(block->m_sName), f);	// 8
			fputs(block->m_sName, f);	// 12

			// 3
			fWriteByte('S', f);	// 12 + len
			fWriteDword(12, f);	// 13 + len
			fputs("Model::Scene", f);	// 17 + len
										// 29 + len
		}
		else if (block->m_pChild)
		{
			WriteConnectionsFBX(f, block->m_pChild);
		}
		block = block->m_pNext;
	}
}

bool B3D::SaveToFBX()
{
	elementsCount = 0;
	char filename[512];
	strcpy(filename, m_sName);
	strcat(filename, ".fbx");

	FILE *f = fopen(filename, "wb");
	if (!f)
		return false;

	fputs("Kaydara FBX Binary  ", f);
	fWriteByte(0x00, f); //
	fWriteByte(0x1A, f); // 
	fWriteByte(0x00, f); // 
	fWriteDword(6100, f); // fileVer

	size_t defPos = ftell(f);
	fWriteDword(defPos + 129, f);	// 0
	fWriteDword(0, f); // numProps	// 4
	fWriteDword(0, f); // propsLen	// 8
	fWriteByte(11, f); // nameLen	// 12
	fputs("Definitions", f);		// 13
	{
		fWriteDword(defPos + 47, f);	// 24
		fWriteDword(1, f); // numProps	// 28
		fWriteDword(5, f); // propsLen	// 32
		fWriteByte(5, f); // nameLen	// 36
		fputs("Count", f);				// 37
		fWriteByte('I', f);				// 42
		fWriteDword(1, f);				// 43

		fWriteDword(defPos + 116, f);	// 47
		fWriteDword(1, f); // numProps	// 51
		fWriteDword(10, f); // propsLen	// 55
		fWriteByte(10, f); // nameLen	// 59
		fputs("ObjectType", f);			// 60
		fWriteByte('S', f);				// 70
		fWriteDword(5, f);				// 71
		fputs("Model", f);				// 75
		{
			fWriteDword(defPos + 103, f);	// 80
			fWriteDword(1, f); // numProps	// 84
			fWriteDword(5, f); // propsLen	// 88
			fWriteByte(5, f); // nameLen	// 92
			fputs("Count", f);				// 93
			fWriteByte('I', f);				// 98
			fWriteDword(0, f);				// 99
		}
		for (int i = 0; i < 13; i++)		// 103
		{
			fWriteByte(0, f);
		}
	}
	for (int i = 0; i < 13; i++)			// 116
	{
		fWriteByte(0, f);
	}

	size_t objPos = ftell(f);				// 129

	fWriteDword(0, f); // endOffset
	fWriteDword(0, f); // numProps
	fWriteDword(0, f); // propsLen
	fWriteByte(7, f); // nameLen

	fputs("Objects", f);

	m_pBlocks->WriteToFBX(f);

	for (int i = 0; i < 13; i++)
	{
		fWriteByte(0, f);
	}

	size_t endpos = ftell(f);
	fseek(f, objPos, SEEK_SET);
	fWriteDword(endpos, f);
	fseek(f, endpos, SEEK_SET);

	fseek(f, defPos + 99, SEEK_SET);
	fWriteDword(elementsCount, f);
	fseek(f, endpos, SEEK_SET);
	
	size_t connPos = ftell(f);
	fWriteDword(0, f); // endOffset
	fWriteDword(0, f); // numProps
	fWriteDword(0, f); // propsLen
	fWriteByte(11, f); // nameLen
	fputs("Connections", f);

	size_t connDataPos = ftell(f);

	WriteConnectionsFBX(f, m_pBlocks);

	size_t endDataPos = ftell(f);
	fseek(f, connPos, SEEK_SET);
	fWriteDword(endDataPos + 13, f);
	fseek(f, endDataPos, SEEK_SET);

	for (int i = 0; i < 13; i++)
	{
		fWriteByte(0, f);
	}
	
	for (int i = 0; i < 13; i++)
	{
		fWriteByte(0, f);
	}
	fclose(f);
}

bool RES::SaveToMtl(const char *path)
{
	char filename[512];
	strcpy(filename, path);
	strcat(filename, ".mtl");

	FILE *f = fopen(filename, "w");
	if (!f)
		return false;

	for (int i = 0; i < m_iMaterialsCount; i++)
	{
		fprintf(f, "newmtl %s\n", m_pMaterials[i].m_sName);
		//fprintf(f, "Kd %f, %f, %f\n", m_pMaterials[i].m_fDiffuse, m_pMaterials[i].m_fDiffuse, m_pMaterials[i].m_fDiffuse);
		//fprintf(f, "Ks %f, %f, %f\n", m_pMaterials[i].m_fGlossiness, m_pMaterials[i].m_fGlossiness, m_pMaterials[i].m_fGlossiness);
		
		if (m_pMaterials[i].m_dTextureId > -1)
		{
			fprintf(f, "map_Kd %s\\%s.tga\n", m_sName, m_pTextureFiles[m_pMaterials[i].m_dTextureId-1].m_sName);
		}
	}
	fclose(f);
}

int LoadFolderB3D()
{
	char Path[500];
	int count = 0;

	WIN32_FIND_DATA ffd;
	memset(&ffd, 0, sizeof(ffd));
	GetCurrentDirectory(500, Path);
	strcpy(tmp, Path);
	strcat(tmp, "\\*.b3d");
	HANDLE handle = FindFirstFile(tmp, &ffd);
	if (handle != INVALID_HANDLE_VALUE)
	{
		WinTreeItem *item = g_pTree->AddItem(ffd.cFileName, 0, 2);
		item->SetCallback(CModel::OnWinTreeEvent);
		B3D::Load(ffd.cFileName, item);
	}
	while(FindNextFile(handle, &ffd))
	{
		WinTreeItem *item = g_pTree->AddItem(ffd.cFileName, 0, 2);
		item->SetCallback(CModel::OnWinTreeEvent);
		B3D::Load(ffd.cFileName, item);
	}
	FindClose(handle);
	return 1;
}

int LoadFolderRES()
{
	char Path[500];
	int count = 0;

	WIN32_FIND_DATA ffd;
	memset(&ffd, 0, sizeof(ffd));
	GetCurrentDirectory(500, Path);
	strcpy(tmp, Path);
	strcat(tmp, "\\*.res");
	HANDLE handle = FindFirstFile(tmp, &ffd);
	if (handle != INVALID_HANDLE_VALUE)
	{
		RES *file = RES::Load(ffd.cFileName);
		WinTreeItem *item = g_pTree->AddItem(ffd.cFileName, (DWORD)file);
		item->SetData((DWORD)file, 1);
		for(int i = 0; i < file->m_iTextureFilesCount; i++)
			item->AddChild(file->m_pTextureFiles[i].m_sName, (DWORD)&file->m_pTextureFiles[i], OnWinTreeTextureEvent);
	}
	while(FindNextFile(handle, &ffd))
	{
		RES *file = RES::Load(ffd.cFileName);
		WinTreeItem *item = g_pTree->AddItem(ffd.cFileName, (DWORD)file);
		item->SetData((DWORD)file, 1);
		for(int i = 0; i < file->m_iTextureFilesCount; i++)
			item->AddChild(file->m_pTextureFiles[i].m_sName, (DWORD)&file->m_pTextureFiles[i], OnWinTreeTextureEvent);
	}
	FindClose(handle);
	return 1;
}

char *fgets0(FILE *f, char *buf)
{
	if(!buf || !f)
		return 0;

	int i = 0;
	while(fread(&buf[i], 1, 1, f))
	{
		if(buf[i] == 0)
			break;
		i++;
	}
	buf[i] = 0;
	if(i == 0)
		return 0;

	return buf;
}

RES::RES()
{
	memset(this, 0, sizeof(RES));
}

RES::~RES()
{
	if(m_pTextureFiles)
		delete[] m_pTextureFiles;
}

B3D::~B3D()
{
	//m_
}

RES *RES::Load(char *filename)
{
	RES *res = new RES();
	if(!res)
		return 0;

	GetFileName(filename, res->m_sName);
	//strcpy(res->m_sName, filename);

	FILE *f = fopen(filename, "rb");
	if(!f)
	{
		res->~RES();
		return 0;
	}

	char section[256];
	char sectionName[64];
	int elementCount;

	while(fgets0(f, section))
	{
		// Узнаем имя секции и кол-во элементов в ней
		// Для начала разделим имя секции и кол-во элементов
		int i = 0;
		for(i = 0; section[i] && section[i] != ' '; i++)
		{
			sectionName[i] = section[i];
		}
		sectionName[i] = 0;
		if(section[i] == 0)
			break;

		elementCount = atoi(&section[i+1]);

		if(strcmpi(sectionName, "PALETTEFILES") == 0)
		{
			res->m_iPaletteFilesCount = elementCount;
			char nameBuf[32];

			for(int j = 0; j < elementCount; j++)
			{
				fgets0(f, nameBuf);
				int offset;
				fread(&offset, 4, 1, f);
				fseek(f, offset, SEEK_CUR);
			}
		}
		else if(strcmpi(sectionName, "SOUNDFILES") == 0)
		{
			res->m_iSoundFilesCount = elementCount;
			char nameBuf[32];

			for(int j = 0; j < elementCount; j++)
			{
				fgets0(f, nameBuf);
				int offset;
				fread(&offset, 4, 1, f);
				fseek(f, offset, SEEK_CUR);
			}
		}
		else if(strcmpi(sectionName, "SOUNDS") == 0)
		{
			res->m_iSoundCount = elementCount;
			char nameBuf[64];

			for(int j = 0; j < elementCount; j++)
			{
				fgets0(f, nameBuf);
				strtok(nameBuf, " ");
				char *soundFileId = strtok(NULL, " ");
			}
		}
		else if(strcmpi(sectionName, "EFFECTS") == 0)
		{
			res->m_iEffectCount = elementCount;
			char nameBuf[64];
		}
		else if(strcmpi(sectionName, "BACKFILES") == 0)
		{
			res->m_iBackFilesCount = elementCount;
			char nameBuf[32];

			for(int j = 0; j < elementCount; j++)
			{
				fgets0(f, nameBuf);
				int offset;
				fread(&offset, 4, 1, f);
				fseek(f, offset, SEEK_CUR);
			}
		}
		else if(strcmpi(sectionName, "MASKFILES") == 0)
		{
			res->m_iMaskFilesCount = elementCount;
			char nameBuf[32];

			for(int j = 0; j < elementCount; j++)
			{
				fgets0(f, nameBuf);
				int offset;
				fread(&offset, 4, 1, f);
				fseek(f, offset, SEEK_CUR);
			}
		}
		else if(strcmpi(sectionName, "TEXTUREFILES") == 0)
		{
			res->m_iTextureFilesCount = elementCount;

			res->m_pTextureFiles = new ResTexture[elementCount];
			for(int j = 0; j < elementCount; j++)
			{
				int start, offset;
				fgets0(f, tmp);
				fread(&offset, 4, 1, f);
				start = ftell(f);

				res->m_pTextureFiles[j].Load(f, tmp);
				res->m_pTextureFiles[j].m_pRes = res;
				
				fseek(f, start + offset, SEEK_SET);
			}
		}
		else if(strcmpi(sectionName, "COLORS") == 0)
		{
			res->m_iColorsCount = elementCount;

			res->m_pColors = new float[elementCount];
			for(int j = 0; j < elementCount; j++)
			{
				fgets0(f, tmp);
				res->m_pColors[j] = atof(tmp);
			}
		}
		else if(strcmpi(sectionName, "MATERIALS") == 0)
		{
			res->m_iMaterialsCount = elementCount;

			res->m_pMaterials = new ResMaterial[elementCount];

			for(int j = 0; j < elementCount; j++)
			{
				fgets0(f, tmp);
				res->m_pMaterials[j].m_dTextureId = -1;
				res->m_pMaterials[j].m_dColorId = -1;
				res->m_pMaterials[j].m_dTextureType = -1;
				res->m_pMaterials[j].m_dTextureTypeData = -1;
				res->m_pMaterials[j].m_dCoord = 0;
				res->m_pMaterials[j].m_dAtt = -1;
				res->m_pMaterials[j].m_bNoTile = false;
				res->m_pMaterials[j].m_bNoZ = false;
				res->m_pMaterials[j].m_bNoF = false;
				res->m_pMaterials[j].m_fDiffuse = 0.f;
				res->m_pMaterials[j].m_fGlossiness = 0.f;
				res->m_pMaterials[j].m_fTransparency = 1.f;

				char *name = strtok(tmp, " ");
				strcpy(res->m_pMaterials[j].m_sName, name);
				while(true)
				{
					char *param = strtok(NULL, " ");
					if(!param)
						break;

					if(strcmpi(param, "col") == 0)
					{
						char *val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_dColorId = atoi(val);
					}
					else if(strcmpi(param, "transp") == 0)
					{
						char *val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_fTransparency = atof(val);
					}
					else if(strcmpi(param, "noz") == 0)
					{
						res->m_pMaterials[j].m_bNoZ = true;
					}
					else if(strcmpi(param, "nof") == 0)
					{
						res->m_pMaterials[j].m_bNoF = true;
					}
					else if(strcmpi(param, "notile") == 0)
					{
						res->m_pMaterials[j].m_bNoTile = true;
					}
					else if(strcmpi(param, "env") == 0)
					{
						char *val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_fDiffuse = atof(val);

						val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_fGlossiness = atof(val);
					}
					else if(strcmpi(param, "coord") == 0)
					{
						char *val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_dCoord = atoi(val);
					}
					else if(strcmpi(param, "tex") == 0)
					{
						char *val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_dTextureId = atoi(val);
					}
					else if(strcmpi(param, "ttx") == 0)
					{
						char *val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_dTextureId = atoi(val);
					}
					else if(strcmpi(param, "att") == 0)
					{
						char *val = strtok(NULL, " ");
						if(!val)
							break;
						res->m_pMaterials[j].m_dAtt = atoi(val);
					}

				}
			}
		}
	}
	
	fclose(f);
	return res;
}

ResTexture::ResTexture()
{
	memset(this, 0, sizeof(ResTexture));
}

ResTexture::~ResTexture()
{
	if(m_pData)
		free(m_pData);//delete[] m_pData;

	if(m_bLoaded)
		glDeleteTextures(1, &m_dTexture);
}

char *GetFileNameFromPath(char *path, char *buf)
{
	for(int i = strlen(path) - 1; i >= 0; i--)
	{
		if(path[i] == '.')
		{
			int j = i;
			for(j = i; j >= 0; j--)
			{
				if(path[j] == '/' || path[j] == '\\')
					break;
			}
			strncpy(buf, &path[j+1], i-j-1);
			buf[i-j-1] = 0;
			break;
		}
	}
	return buf;
}

void ResTexture::LoadFromTarga(Targa *img, DWORD type)
{
	m_iWidth = img->m_Header.width;
	m_iHeight = img->m_Header.height;
	if(type == IMAGE_RGB_565)
	{
		m_bDepth = 16;
		m_bFormat = 1;
	}
	else if(type == IMAGE_RGBA_4444)
	{
		m_bDepth = 16;
		m_bFormat = 5;
	}
	else if(type == IMAGE_RGB_888)
	{
		m_bDepth = 24;
		m_bFormat = 1;
	}
	else if(type == IMAGE_RGBA_8888)
	{
		m_bDepth = 32;
		m_bFormat = 5;
	}
	else
		return;

	if(m_pData)
		delete[] m_pData;

	m_pData = new BYTE[m_iWidth*m_iHeight*(m_bDepth/8)];
	memset(m_pData, 0, m_iWidth*m_iHeight*(m_bDepth/8));

	img->GetData(m_pData, type);
}

void ResTexture::Load(FILE *f, char *path)
{
	int nop, offset, start, blockOffset;
	char __offset = 0, flag1 = 0, flag2 = 0;
	char hasColor = 0, bitColor = 0;
	WORD paletteCount = 0;

	struct {
		BYTE r;
		BYTE g;
		BYTE b;
	} palette[256];

	m_iWidth = 0;
	m_iHeight = 0;
	m_bDepth = 0;

	strcpy(m_sPath, path);
	GetFileNameFromPath(m_sPath, m_sName);

	start = ftell(f);

	fread(&__offset, 1, 1, f);
	fread(&hasColor, 1, 1, f);
	fread(&flag2, 1, 1, f);
	fread(&nop, 1, 2, f);
	fread(&paletteCount, 2, 1, f);
	fread(&bitColor, 1, 1, f);

	fread(&nop, 4, 1, f);

	fread(&m_iWidth, 2, 1, f);
	fread(&m_iHeight, 2, 1, f);
	fread(&m_bDepth, 1, 1, f);

	fread(&flag1, 1, 1, f);

	int endOfHeader = ftell(f);
	int loffStart = 0;
	if (__offset <= 0)
	{
		fseek(f, __offset + endOfHeader, SEEK_SET);
		if (hasColor)
		{
			for (int i = 0; i < paletteCount; i++)
			{
				fread(&palette[i].r, 1, 1, f);
				fread(&palette[i].g, 1, 1, f);
				fread(&palette[i].b, 1, 1, f);
			}
		}


		int bitDepth = m_bDepth / 8;
		void *data = malloc(m_iWidth*m_iHeight*bitDepth);
		m_pData = malloc(m_iWidth*m_iHeight*3);

		if (flag2 == 1)
		{
			if (bitDepth == 1)
			{
				for (int i = 0; i < m_iHeight; i++)
				{
					int j = flag1 & 0x20 ? i : m_iHeight - i - 1;
					fread(&((char*)data)[m_iWidth * j], m_iWidth, 1, f);
				}
				for (int i = 0; i < m_iHeight; i++)
				{
					for (int k = 0; k < m_iWidth; k++)
					{
						((char*)m_pData)[m_iWidth * i * 3 + k * 3] = palette[((char*)data)[m_iWidth * i + k]].r;
						((char*)m_pData)[m_iWidth * i * 3 + k * 3 + 1] = palette[((char*)data)[m_iWidth * i + k]].g;
						((char*)m_pData)[m_iWidth * i * 3 + k * 3 + 2] = palette[((char*)data)[m_iWidth * i + k]].b;
					}
				}
			}
		}

		free(data);
	}
	else
	{
		fread(&nop, 4, 1, f); // LOFF
		fread(&nop, 4, 1, f); // 4?
		fread(&blockOffset, 4, 1, f); // end of LOFF block
		loffStart = ftell(f);

		fseek(f, start + blockOffset, SEEK_SET); // Seek to other blocks in texture
		while (!feof(f))
		{
			char blockName[4];
			fread(&blockName, 1, 4, f);
			fread(&blockOffset, 4, 1, f); // end of block
			int blockStart = ftell(f);

			if (strncmp(blockName, "LVLS", 4) == 0)
			{
				break;
			}
			else if (strncmp(blockName, "LVLH", 4) == 0)
			{
				break;
			}
			else if (strncmp(blockName, "LEVL", 4) == 0)
			{
				break;
			}
			else if (strncmp(blockName, "LVMP", 4) == 0)
			{
				blockOffset += 2;
			}
			else if (strncmp(blockName, "PFRM", 4) == 0)
			{
				int flag1, flag2, flag3, flag4, flag5, flag6, flag7;
				fread(&flag1, 4, 1, f);
				fread(&flag2, 4, 1, f);
				fread(&flag3, 4, 1, f);
				fread(&flag4, 4, 1, f);
				if (blockOffset > 0x10)
				{
					fread(&flag5, 4, 1, f);
					fread(&flag6, 4, 1, f);
					fread(&flag7, 4, 1, f);
				}

				if (flag3 == 0x1F)
				{
					if (flag2 == 0x3E0)
					{
						if (flag1 == 0x7C00 && !flag4)
							m_bFormat = 3;
					}
					else if (flag2 == 0x7E0 && flag1 == 0xF800 && !flag4)
						m_bFormat = 2;
				}
				else if (flag3 == 0xF)
				{
					if (flag2 == 0xF0 && flag1 == 0xF00 && flag4 == 0xF000)
					{
						m_bFormat = 5;
					}
				}
				else if (flag3 == 0xFF && flag2 == 0xFF00 && flag1 == 0xFF0000)
				{
					if (!flag4)
					{
						m_bFormat = 6;
					}
					else if (flag4 == 0xFF000000)
					{
						m_bFormat = 7;
					}
				}
				else if (flag5 == 0xFF00 && flag6 == 0xFF)
					m_bFormat = 8;
				else
					m_bFormat = 0;
			}
			else if (strncmp(blockName, "ENDR", 4) == 0)
			{
				break;
			}

			fseek(f, blockStart + blockOffset, SEEK_SET);
		}

		fseek(f, loffStart, SEEK_SET);
		//if(m_bFormat == 5)
		//m_pData = new BYTE[m_iWidth*m_iHeight*4];
		//else
		m_pData = malloc(m_iWidth*m_iHeight*(m_bDepth / 8));//new BYTE[m_iWidth*m_iHeight*(m_bDepth/8)];
		memset(m_pData, 0, m_iWidth*m_iHeight*(m_bDepth / 8));
		//fread(m_pData, 1, m_iWidth*m_iHeight*(m_bDepth/8), f);

		WORD data;
		for (int i = 0; i < m_iWidth*m_iHeight; i++)
		{
			fread(&data, 1, m_bDepth / 8, f);
			//if(m_bFormat == 5)
			//((DWORD *)m_pData)[i] = GetRGBAFrom4444(data);
			//else

			if (m_bFormat == 5)
			{
				BYTE a, r, g, b;
				a = (data & 0xF000) >> 12;
				b = (data & 0xF00) >> 8;
				g = (data & 0xF0) >> 4;
				r = (data & 0xF);

				data = (a << 12) | (r << 8) | (g << 4) | b;
			}
			else if (m_bFormat == 3)
			{
				data = Get565From555(data);
			}
			((WORD *)m_pData)[i] = data;
		}

	}
}

char *ResTexture::GetName()
{
	return m_sName;
}

int ResTexture::SaveToFile(char *path)
{
	int format = 0;
	if(m_bDepth == 16)
	{
		if(m_bFormat == 5)
			format = IMAGE_RGBA_4444;
		else
			format = IMAGE_RGB_565;
	}
	else if(m_bDepth == 24)
	{
		format = IMAGE_RGB_888;
	}
	else
		format = IMAGE_RGBA_8888;

	Targa *img = Targa::LoadTargaImage(m_iWidth, m_iHeight, format, m_pData);
	int result = img->SaveTargaImage(path);
	img->Destroy();
	return result;
}

GLuint ResTexture::glLoad()
{
	if(m_bLoaded)
		return m_dTexture;
	
	glGenTextures(1, &m_dTexture);
	glBindTexture(GL_TEXTURE_2D, m_dTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	if(m_bDepth == 8)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iWidth, m_iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pData);
	else if(m_bDepth == 16 && m_bFormat == 5)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iWidth, m_iHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV, m_pData);
	else if(m_bDepth == 16)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iWidth, m_iHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, m_pData);
	else if(m_bDepth == 24)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iWidth, m_iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pData);
	else if(m_bDepth == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iWidth, m_iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pData);

	m_bLoaded = true;
	return m_dTexture;
}

void ResTexture::glRelease()
{
	if(!m_bLoaded)
		return;

	glDeleteTextures(1, &m_dTexture);
	m_bLoaded = false;
}

void ResTexture::glSetActive()
{
	if(!m_bLoaded)
		glLoad();
	glBindTexture(GL_TEXTURE_2D, m_dTexture);
}

CBlock::CBlock()
{
	m_dType = 0;
	m_pChild = 0;
	m_pChildCount = 0;
	m_pNext = 0;
	m_pTreeItem = 0;
	m_bIsModel = false;
	memset(m_sName, 0, 32);
}

CModel::CModel()
{
	m_bIsModel = true;
}

void CBlock::Destroy()
{
	
}

CBlock::~CBlock()
{
	if(m_pChild)
	{
		CBlock *next = m_pChild;
		while(next)
		{
			next->~CBlock();
			next = next->m_pNext;
		}
	}
}

CModel::~CModel()
{
	if(m_pVertices)
		delete[] m_pVertices;
	if(m_pElements)
		delete[] m_pElements;
}

extern Node<CModel> *currentGlModel;
extern ResTexture *currentTex;

void CALLBACK ShowUpAllWinTree(WinTreeItem *item)
{
	DWORD type;
	if(item->GetData(&type) && type != 2)
	{
		if(((CModel*)item->GetData())->m_dType == 0x25 || ((CModel*)item->GetData())->m_dType == 0x7 || ((CModel*)item->GetData())->m_dType == 0x24)
		{
			if(currentGlModel->GetNode((CModel*)item->GetData()) == 0)
				currentGlModel->GetLast()->next = new Node<CModel>((CModel*)item->GetData(), 0);
		}
	}
}

void CALLBACK HideUpAllWinTree(WinTreeItem *item)
{
	DWORD type;
	if(item->GetData(&type) && type != 2)
	{
		CModel *model = (CModel*)item->GetData();
		if(model->m_dType == 0x25 || model->m_dType == 0x7 || model->m_dType == 0x24)
			currentGlModel->Delete((CModel*)item->GetData());
	}
}
extern HWND g_hPos;
void CALLBACK CModel::OnWinTreeEvent(WinTree *tree, WinTreeItem *item, DWORD eventId, DWORD flags)
{
	if(!item)
		return;
	DWORD type;

	if(eventId == TVN_SELCHANGED)
	{
		if(item->GetData(&type) && type != 2)
		{
			CBlock *block = (CBlock*)item->GetData();
			sprintf(tmp, "PosX: %f\nPosY: %f\nPosZ: %f", block->m_fX, block->m_fY, block->m_fZ);
			UI_SetLabelText(g_hPos, tmp);
			//sprintf(tmp, "PosY: %f", ((CModel*)item->GetData())->m_fY);
			//UI_SetLabelText(g_hPosY, tmp);
			//sprintf(tmp, "PosZ: %f", ((CModel*)item->GetData())->m_fZ);
			//UI_SetLabelText(g_hPosZ, tmp);
		}
	}

	if(eventId == NM_CLICK && flags & TVHT_ONITEMSTATEICON)
	{
		if(currentTex)
			currentTex->glRelease();

		currentTex = 0;
		if(currentGlModel)
		{
			if(!TreeView_GetCheckState(tree->m_hWnd, item->m_hItem))
			{
				//TreeView_SetCheckState(tree->m_hWnd, item->m_hItem, 1);
				item->ForAllChildren(ShowUpAllWinTree);
				if(item->GetData(&type) == 0 || type == 2)
					return;
				currentGlModel->GetLast()->next = new Node<CModel>((CModel*)item->GetData(), 0);
			}
			else
			{
				//TreeView_SetCheckState(tree->m_hWnd, item->m_hItem, 0);
				item->ForAllChildren(HideUpAllWinTree);
				if(item->GetData(&type) == 0 || type == 2)
					return;
				currentGlModel->Delete((CModel*)item->GetData());
			}
		}
		//currentGlModel = (CModel *)item->GetData();
		//if(currentGlModel)
			//CCamera::g_pCamera->SetPos(currentGlModel->m_fX, currentGlModel->m_fY, currentGlModel->m_fZ);
	}
}

void CALLBACK OnWinTreeResEvent(WinTree *tree, WinTreeItem *item, DWORD eventId, DWORD flags)
{

}

CElement::CElement()
{
	//polyCount = 0;
	//polys = 0;
}

CElement::~CElement()
{
	/*if(polys)
	{
		if(polyCount)
		{
			for(int i = 0; i < polyCount; i++)
			{
				if(polys[i].vertices)
					delete[] polys[i].vertices;
			}
		}
		delete[] polys;
	}*/
}

ResTexture *RES::GetByName(char *name)
{
	for(int i = 0; i < m_iMaterialsCount; i++)
	{
		int id = m_pMaterials[i].m_dTextureId;
		if(id > -1)
		{
			if(strcmpi(m_pMaterials[i].m_sName, name) == 0)
				return &m_pTextureFiles[id-1];
		}
	}
	return 0;
}