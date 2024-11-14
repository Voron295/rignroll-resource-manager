#include "UI.h"
#include <GL/gl.h>
#include <iostream>
#include "Targa.h"
#include <vector>
int LoadFolderB3D();
int LoadFolderRES();

class CBlock
{
public:
	class B3D	*m_pB3D;
	char	m_sName[32];
	int		m_dType;
	bool	m_bIsModel;
	int		m_pChildCount;
	CBlock	*m_pChild;
	CBlock	*m_pNext;
	WinTreeItem *m_pTreeItem;

	float m_fX;
	float m_fY;
	float m_fZ;
	float m_fAngle;

	CBlock();
	static CBlock *Read(FILE *file, WinTreeItem *item, class B3D *b3d);
	virtual bool WriteTo3DS(FILE *f);
	virtual bool WriteToObj(FILE **f, bool first = false);
	virtual bool WriteToFBX(FILE *f);
	void Destroy();
	CBlock *GetLast();
	virtual ~CBlock();
};

struct Vertex
{
	float x;
	float y;
	float z;
	float u;
	float v;
};

struct CVector
{
	float x;
	float y;
	float z;
};

struct UV
{
	float u;
	float v;
};

struct Poly
{
	int		materialId;
	int		vertsCount;
	std::vector<int> vertices;
	DWORD	internalFormat;
};

class CElement
{
public:
	//int		polyCount;
	std::vector<Poly>	polys;
	int		*triangles;

	CElement();
	~CElement();

	CElement *Read(FILE *f);
	void ReadFaces(FILE *f, int count);
};

class CModel : public CBlock
{
public:
	int		m_iVerticesCount;
	Vertex	*m_pVertices;
	CVector *m_pVerticesPos;
	UV		*m_pUVs;
	int		m_iElementCount;
	CElement *m_pElements;
	int		m_pGlList;
	int		m_dSubType;

	CModel();
	static CModel *Read(FILE *f, WinTreeItem *item, DWORD type, class B3D *b3d);
	virtual bool WriteTo3DS(FILE *f);
	virtual bool WriteToObj(FILE **f, bool first = false);
	virtual bool WriteToFBX(FILE *f);
	static void CALLBACK OnWinTreeEvent(WinTree *tree, WinTreeItem *item, DWORD eventId, DWORD flags);
	void GenerateGlModel();
	void glRender();
	virtual ~CModel();
};

class B3D
{
public:
	char		m_sName[64];
	class RES	*m_pRes;
	CBlock		*m_pBlocks;
	int			m_iMaterialsCount;
	char		**m_sMaterials;
	
	static B3D *Load(char *filename, WinTreeItem *item = 0);
	bool ReadBlock(FILE *file, WinTreeItem *item);
	bool SaveTo3DS();
	bool SaveToObj(const char *path);
	bool SaveToFBX();
	~B3D();
};

class ResTexture
{
public:
	static int g_iGLTextureCount;

	class RES *m_pRes;
	char m_sName[64];
	char m_sPath[64];
	int m_iWidth;
	int m_iHeight;
	BYTE m_bDepth;
	BYTE m_bFormat;
	GLvoid *m_pData;
	GLuint m_dTexture;
	bool m_bLoaded;


	ResTexture();
	~ResTexture();
	void Load(FILE *f, char *name);
	void LoadFromTarga(Targa *img, DWORD type);
	int SaveToFile(char *path);
	char *GetName();
	GLuint glLoad();
	void glRelease();

	void glSetActive();
};

struct ResMaterial
{
	char	m_sName[32];
	int		m_dTextureId;
	int		m_dColorId;
	int		m_dTextureType;
	int		m_dTextureTypeData;
	int		m_dCoord;
	int		m_dAtt;
	bool	m_bNoZ;
	bool	m_bNoF;
	bool	m_bNoTile;
	float	m_fDiffuse;
	float	m_fGlossiness;
	float	m_fTransparency;
};

struct ResPalette
{
	char name[64];

};

struct ResSound
{
	char name[64];
};

struct ResMask
{
	char name[64];
};

class RES
{
private:
	RES();
	~RES();

public:
	char	m_sName[128];
	int		m_iPaletteFilesCount;
	int		m_iSoundFilesCount;
	int		m_iSoundCount;
	int		m_iEffectCount;
	int		m_iBackFilesCount;
	int		m_iMaskFilesCount;
	int		m_iTextureFilesCount;
	int		m_iColorsCount;
	int		m_iMaterialsCount;

	float	*m_pColors;
	ResMaterial *m_pMaterials;
	ResTexture *m_pTextureFiles;

	static RES *Load(char *filename);
	ResTexture *GetByName(char *name);
	bool SaveToMtl(const char *path);
	void Destroy();
};