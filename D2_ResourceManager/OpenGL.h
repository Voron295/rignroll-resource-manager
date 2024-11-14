// OpenGL.h
// Объявление внешних функций

#ifndef _OpenGL_h_
#define _OpenGL_h_

#include <Windows.h>
int InitOpenGL(HWND hWnd);
void ReleaseOpenGL();
void DrawFrame();                          //Рисуем кадр

#endif //_OpenGL_h_