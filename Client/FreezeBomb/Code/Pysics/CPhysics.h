#pragma once
#include <math.h>
#include "Precision.h"

namespace Physics
{
	extern real sleepEpsilon;

	void setSleepEpsilon(real value);

	real getSleepEpsilon();

	//Physics ���� �����̽� �ȿ� ���� �Ǿ� �־ 
	//XMFLOAT3�� �浹�� �Ͼ�� ����
	class Vector3
	{
	public:
		real x;

		real y;

		real z;

	private:
		// padding to ensure 4 word alignment
		real pad;

	public:
		//default constructor creates a zero vector
		Vector3() : x(0), y(0), z(0) {}

		Vector3(const real x, const real y, const real z)
			:x(x), y(y), z(z) {}

		const static Vector3 GRAVITY;
        const static Vector3 HIGH_GRAVITY;
        const static Vector3 UP;
        const static Vector3 RIGHT;
        const static Vector3 OUT_OF_SCREEN;
        const static Vector3 X;
        const static Vector3 Y;
        const static Vector3 Z;
	};
}