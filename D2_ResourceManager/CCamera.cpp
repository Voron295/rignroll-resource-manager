#include "CCamera.h"
#include <math.h>
#include "UI.h"
#include "UI_Elements.h"

extern char tmp[512];
extern HWND g_hCamPos;
//extern HWND g_hCamPosY;
//extern HWND g_hCamPosZ;

CCamera *CCamera::g_pCamera = 0;

void CCamera::Init()
{
	if(g_pCamera)
		g_pCamera->~CCamera();

	g_pCamera = new CCamera();
	g_pCamera->m_pMatrix = new CMatrix();
	g_pCamera->SetPos(0, 0, 0);
	g_pCamera->SetRot(0, 0, 0);
	g_pCamera->m_fAngleX = 0;
	g_pCamera->m_fAngleZ = 0;
	g_pCamera->isClicked = false;
}

void CCamera::SetPos(float x, float y, float z)
{
	m_fX = x;
	m_fY = y;
	m_fZ = z;
}

void CCamera::SetRot(float x, float y, float z)
{
	/*x = x * PI / 180.f;
	y = y * PI / 180.f;
	z = z * PI / 180.f;

	m_fTopX = cosf(z);
	m_fTopY = sinf(z);
	m_fTopZ = 0;

	m_fAtX = 0;
	m_fAtY = 0;
	m_fAtZ = 1;*/
	m_pMatrix->Rotate(x, y, z);
	//m_fTopX = m_pMatrix->m_fTopX;
}

void CCamera::glSet()
{
	gluLookAt(m_fX, m_fY, m_fZ, m_fX + m_pMatrix->m_fTopX, m_fY + m_pMatrix->m_fTopY, m_fZ + m_pMatrix->m_fTopZ, 
		m_pMatrix->m_fAtX, m_pMatrix->m_fAtY, m_pMatrix->m_fAtZ);
	sprintf(tmp, "CamPosX: %f\nCamPosY: %f\nCamPosZ: %f", m_fX, m_fY, m_fZ);
	UI_SetLabelText(g_hCamPos, tmp);
}

extern HWND g_hGlWnd;
extern HWND g_hWnd;
extern WinTree *g_pTree;
extern int g_iGlWidth;
extern int g_iGlHeight;

void CCamera::Process()
{
	if (GetForegroundWindow() != g_hWnd && GetForegroundWindow() != g_hGlWnd)
		return;

	int offsetX = 0;
	int offsetY = 0;
	POINT pt;

	if(isClicked)
	{
		if((GetKeyState(VK_LBUTTON) & 0x8000) == 0)
		{
			isClicked = false;
			ShowCursor(1);
		}

	}
	else
	{
		if(GetFocus() != g_hWnd && GetFocus() != g_pTree->m_hWnd && GetFocus() != g_hGlWnd)
			return;

		if(GetKeyState(VK_LBUTTON) & 0x8000)
		{
			GetCursorPos(&pt);
			ScreenToClient(g_hGlWnd, &pt);
			if(pt.x >= 0 && pt.y >= 0 && pt.x <= g_iGlWidth && pt.y <= g_iGlHeight)
			{
				isClicked = true;
				pt.x = g_iGlWidth / 2;
				pt.y = g_iGlHeight / 2;
				ClientToScreen(g_hGlWnd, &pt);
				SetCursorPos(pt.x, pt.y);
				SetFocus(g_hGlWnd);
				ShowCursor(0);
			}
		}
	}

	if(isClicked)
	{
		GetCursorPos(&pt);
		ScreenToClient(g_hGlWnd, &pt);
		offsetX = pt.x - g_iGlWidth / 2;
		offsetY = pt.y - g_iGlHeight / 2;
		pt.x = g_iGlWidth / 2;
		pt.y = g_iGlHeight / 2;
		ClientToScreen(g_hGlWnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}

	if(!isClicked)
		return;
	
	float x = 0;
	float y = 0;
	float z = 0;
	if(GetKeyState('W') & 0x8000)
	{
		y += 1;
	}
	if(GetKeyState('S') & 0x8000)
	{
		y -= 1;
	}
	if(GetKeyState('A') & 0x8000)
	{
		x -= 1;
	}
	if(GetKeyState('D') & 0x8000)
	{
		x += 1;
	}
	if(GetKeyState('E') & 0x8000)
	{
		z += 1;
	}
	if(GetKeyState('Q') & 0x8000)
	{
		z -= 1;
	}
	if(GetKeyState(VK_SHIFT) & 0x8000)
	{
		x *= 5;
		y *= 5;
		z *= 5;
	}
	if(GetKeyState(VK_CONTROL) & 0x8000)
	{
		x /= 20;
		y /= 20;
		z /= 20;
	}

	m_fAngleZ -= offsetX/4.f;
	for(;m_fAngleZ < 0.f; m_fAngleZ += 360.f);
	for(;m_fAngleZ > 360.f; m_fAngleZ -= 360.f);

	m_fAngleX -= offsetY/4.f;
	for(;m_fAngleX < 0.f; m_fAngleX += 360.f);
	for(;m_fAngleX > 360.f; m_fAngleX -= 360.f);

	SetRot(m_fAngleX, 0.0, m_fAngleZ);
	SetPos(m_fX + x * m_pMatrix->m_fTopY + y * m_pMatrix->m_fTopX, m_fY - x * m_pMatrix->m_fTopX + y * m_pMatrix->m_fTopY, m_fZ + z);
}