// OpenGL.cpp
// http://www.gamedev.ru

// Урок: http://www.gamedev.ru/code/articles/?id=4268
// Автор: Sergey Watkin

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glaux.h>
//#include <GL/glut.h>

#include "OpenGL.h"
#include "KingStructures.h"
#include "CCamera.h"
//#include "CNode.h"

int InitPixelFormat(HDC hdc);
void InitSettings();

namespace{
  HWND hWnd;
  HDC   hDC;
  HGLRC hRC;
}

extern int g_iGlWidth;
extern int g_iGlHeight;

int InitOpenGL(HWND _hWnd)
{
  hWnd = _hWnd;
  hDC = GetDC(hWnd);
  if(!InitPixelFormat(hDC))
    return 0;

  hRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hRC);

  InitSettings();
  
  return 1;
}

ResTexture *currentTex = 0;
extern WinTree *g_pTree;

void InitSettings()
{
	CCamera::Init();
	CCamera::g_pCamera->SetPos(0.0, -5.0f, 0.0);
}

void Init3D()
{
	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0f, 450/560, 1.0f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef( 450 / 2.f, 560 / 2.f, 0.f );*/

	glMatrixMode( GL_PROJECTION );            // Выбор матрицы проекций
    glLoadIdentity();              // Сброс матрицы проекции
 
    // Вычисление соотношения геометрических размеров для окна
    gluPerspective( 45.0f, (GLfloat)g_iGlWidth/(GLfloat)g_iGlHeight, 0.1f, 10000.0f );
 
    glMatrixMode( GL_MODELVIEW );            // Выбор матрицы вида модели
    glLoadIdentity();              // Сброс матрицы вида модели
	
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth( 1.0f );              // Разрешить очистку буфера глубины
	glEnable( GL_DEPTH_TEST );            // Разрешить тест глубины
    glDepthFunc( GL_LEQUAL );
}

void Init2D()
{
	glViewport( 0, 0, g_iGlWidth, g_iGlHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glPushMatrix();
	glOrtho( 0, g_iGlWidth, g_iGlHeight, 0, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void ReleaseOpenGL()
{
  if(hRC)
  {
    wglMakeCurrent(hDC, 0);
    wglDeleteContext(hRC);
    hRC = 0;
  }
  if(hDC)
  {
    ReleaseDC(hWnd, hDC);
    hDC = 0;
  }
}

int InitPixelFormat(HDC hdc)
{
  int pixelformat;
  PIXELFORMATDESCRIPTOR pfd = {0};

  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  if (!(pixelformat = ChoosePixelFormat(hdc, &pfd)))
  {
    //Error: ChoosePixelFormat failed
    return 0;
  }

  if(!SetPixelFormat(hdc, pixelformat, &pfd))
  {
    //Error: SetPixelFormat failed"
    return 0;
  }

  return 1;
}
Node<CModel> *currentGlModel = 0;

void DrawObjects()
{
	
  
  //glEnableClientState(GL_VERTEX_ARRAY);
  
 // glVertexPointer(3,GL_FLOAT,0,pVerts);
  //glColorPointer(3, GL_BYTE, 0, pColors);
  
  

  //glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, pInds);
	//glViewport( 0, 0, width, height );

	CCamera::g_pCamera->Process();

	Init3D();
	glEnable (GL_TEXTURE_2D);
	//glEnable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	CCamera::g_pCamera->glSet();
	
	if(currentGlModel)
	{
		Node<CModel> *next = currentGlModel;
		while(next)
		{
			if(next->data)
				next->data->glRender();
			next = next->next;
		}
	}

	glDisable(GL_TEXTURE_2D);
	//glTranslatef(0,0.0f,-3.0f);
	/*glBegin(GL_TRIANGLES);
	glVertex3f( 0.0f, 1.0f, 0.0f);  // Вверх
	glVertex3f(-1.0f,-1.0f, 0.0f);  // Слева снизу
	glVertex3f( 1.0f,-1.0f, 0.0f);  // Справа снизу
	glEnd();*/
	//glTranslatef(3.0f,0.0f,0.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	

	if(currentTex)
	{
		Init2D();
		glEnable (GL_TEXTURE_2D);
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		currentTex->glSetActive();
		glBegin(GL_QUADS);
	
		glTexCoord2f(0.0, 0.0);
		glVertex2d(0, 0);
		glTexCoord2f(0.0, 1.0);
		glVertex2d(0, currentTex->m_iHeight);
		glTexCoord2f(1.0, 1.0);
		glVertex2d(currentTex->m_iWidth, currentTex->m_iHeight);
		glTexCoord2f(1.0, 0.0);
		glVertex2d(currentTex->m_iWidth, 0);

		glEnd();

		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}
}

void DrawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT);
	DrawObjects();
	SwapBuffers(hDC);
}