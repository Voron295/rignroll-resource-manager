#include <Windows.h>
#include <gl/GLU.h>
#include <gl/GL.h>
#include "CMatrix.h"

class CCamera
{
private:
	CMatrix *m_pMatrix;
	float m_fX;
	float m_fY;
	float m_fZ;

	float m_fTopX;
	float m_fTopY;
	float m_fTopZ;

	float m_fAtX;
	float m_fAtY;
	float m_fAtZ;

	float m_fAngleX;
	float m_fAngleZ;

	bool isClicked;

public:
	static CCamera *g_pCamera;
	static void Init();
	void SetPos(float x, float y, float z);
	void SetRot(float x, float y, float z);
	void glSet();
	void Process();
};