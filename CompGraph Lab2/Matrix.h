#pragma once
#include "Vector.h"
#define _USE_MATH_DEFINES
#include <math.h>

class MyMatrix
{
private:
	float m_matrix[16];

public:
	MyMatrix() :
		m_matrix{ 1.0, 0.0, 0.0, 0.0,
				  0.0, 1.0, 0.0, 0.0,
				  0.0, 0.0, 1.0, 0.0,
				  0.0, 0.0, 0.0, 1.0 } {}

	MyMatrix(float matrix[16]) { memcpy(m_matrix, matrix, 16 * sizeof(float)); }

	const float* get() const { return m_matrix; }

	const float& operator [](int i) const { return m_matrix[i]; }

	MyMatrix operator *(const MyMatrix& m)
	{
		float answ[16];

		for (int i = 0; i < 16; i += 4)
		{
			for (int j = 0; j < 4; j++)
			{
				float ij = 0;

				for (int l = 0, k = j; k < 16; l++, k += 4)
				{
					ij += m_matrix[k] * m[l + i];
				}
				answ[i + j] = ij;
			}
		}
		return MyMatrix(answ);
	}

	static MyMatrix scale(MyMatrix matrix, float x, float y, float z)
	{
		float scale[16]{ x,   0.0, 0.0, 0.0,
						 0.f,   y, 0.f, 0.f,
						 0.f, 0.f,  z,  0.f,
						 0.f, 0.f, 0.f, 1.f
		};

		return matrix * scale;
	}

	static MyMatrix translate(MyMatrix matrix, float x, float y, float z)
	{
		float translate[16]{ 1.0, 0.0, 0.0, 0.0,
							 0.0, 1.0, 0.0, 0.0,
							 0.0, 0.0, 1.0, 0.0,
							 x,   y,   z,   1.0
		};

		return matrix * translate;
	}

	static MyMatrix rotate(MyMatrix matrix, float angle, float x, float y, float z)
	{
		angle *= M_PI / 180.f;
		float c = cos(angle), s = sin(angle);

		float rotate[16]{ x * x * (1.0 - c) + c,     x * y * (1.0 - c) + z * s, x * z * (1.0 - c) - y * s, 0.0,
						  y * x * (1.0 - c) - z * s, y * y * (1.0 - c) + c,     y * z * (1.0 - c) + x * s, 0.0,
					   	  z * x * (1.0 - c) + y * s, z * y * (1.0 - c) - x * s, z * z * (1.0 - c) + c    , 0.0,
						  0.0,                       0.0,                       0.0,                       1.0
		};

		return matrix * rotate;
	}

	static MyMatrix perspProj(float l, float r, float b, float t, float n, float f)
	{
		float perspP[16]{ 2.0 * n / (t - b), 0.0,               0.0,                    0.0,
						  0.0,               2.0 * n / (r - l), 0.0,                    0.0,
						  (r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n),    -1.0,
						  0.0,               0.0,               -2.0 * f * n / (f - n), 0.0
		};

		return MyMatrix(perspP);
	}

	static MyMatrix perspProjFOV(float n, float f, float w, float h, float alph)
	{
		float tan_alph = tanf(alph * M_PI / 180 / 2.0);
		return perspProj(-n * tan_alph, n * tan_alph, -n * w / h * tan_alph, n * w / h * tan_alph, n, f);
	}

	static MyMatrix getView(MyVector E, MyVector C, MyVector u)
	{
		MyVector f = MyVector::normalize(C - E);
		MyVector s = MyVector::normalize(MyVector::cross(f, u));
		MyVector v = MyVector::cross(s, f);

		float M1[16]{ s.X(), v.X(), -f.X(), 0.0,
					  s.Y(), v.Y(), -f.Y(), 0.0,
					  s.Z(), v.Z(), -f.Z(), 0.0,
					  0.0,   0.0,  0.0,  1.0
		};

		float M2[16]{ 1.0,    0.0,    0.0,    0.0,
					  0.0,    1.0,    0.0,    0.0,
					  0.0,    0.0,    1.0,    0.0,
					  -E.X(), -E.Y(), -E.Z(), 1.0
		};

		return MyMatrix(M1) * MyMatrix(M2);
	}

	static float* getInvNotTransp(MyMatrix mat)
	{
		float m[9]{ mat[0], mat[1], mat[2],
					mat[4], mat[5], mat[6],
					mat[8], mat[9], mat[10]
		};

		float minorsMatrix[9]{ m[4] * m[8] - m[5] * m[7], m[5] * m[6] - m[3] * m[8], m[3] * m[7] - m[6] * m[4],
							   m[2] * m[7] - m[1] * m[8], m[0] * m[8] - m[2] * m[6], m[6] * m[1] - m[0] * m[7],
							   m[1] * m[5] - m[4] * m[2], m[3] * m[2] - m[0] * m[5], m[0] * m[4] - m[1] * m[3],
		};

		float det = m[0] * m[4] * m[8] + m[3] * m[7] * m[2] + m[1] * m[5] * m[6]
					- m[6] * m[4] * m[2] - m[7] * m[5] * m[0] - m[1] * m[3] * m[8];

		return minorsMatrix;
	}
};