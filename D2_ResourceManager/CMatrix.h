#ifndef CCMATRIX_CLASS
#define CCMATRIX_CLASS

#include <Windows.h>
#define PI 3.1415926f

class CMatrix
{
public:
	union
	{
		struct
		{
			float m_fRightX;
			float m_fRightY;
			float m_fRightZ;

			float m_fTopX;
			float m_fTopY;
			float m_fTopZ;

			float m_fAtX;
			float m_fAtY;
			float m_fAtZ;
		};
		struct
		{
			float m_fRows[3][3];
		};
		float m_fElements[9];
	};

	CMatrix();
	CMatrix(const CMatrix &m);
	~CMatrix();

	void SetIdentity();
	void Rotate(float x, float y, float z);
	void RotateX(float x);
	void RotateY(float y);
	void RotateZ(float z);

	CMatrix &operator=(CMatrix &right);
	CMatrix &operator*=(CMatrix &right);
	CMatrix &operator*(CMatrix &right);
};

#endif