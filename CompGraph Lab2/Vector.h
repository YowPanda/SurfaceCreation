#pragma once

class MyVector
{
private:
	float inner_vec[3];

public:
	MyVector() : inner_vec{ 0, 0, 0 } {}
	MyVector(float x, float y, float z) : inner_vec{ x, y, z } {}

	const float& X() const { return inner_vec[0]; };
	const float& Y() const { return inner_vec[1]; };
	const float& Z() const { return inner_vec[2]; };

	MyVector operator + (const float c)
	{
		return MyVector(X() + c, Y() + c, Z() + c);
	}

	MyVector operator - (const float c)
	{
		return MyVector(X() - c, Y() - c, Z() - c);
	}

	MyVector operator * (const float c)
	{
		return MyVector(X() * c, Y() * c, Z() * c);
	}

	MyVector operator / (const float c)
	{
		return MyVector(X() / c, Y() / c, Z() / c);
	}

	MyVector operator + (const MyVector& vec)
	{
		return MyVector(X() + vec.X(), Y() + vec.Y(), Z() + vec.Z());
	}

	MyVector operator - (const MyVector& vec)
	{
		return MyVector(X() - vec.X(), Y() - vec.Y(), Z() - vec.Z());
	}

	MyVector operator * (const MyVector& vec)
	{
		return MyVector(X() * vec.X(), Y() * vec.Y(), Z() * vec.Z());
	}

	MyVector operator / (const MyVector& vec)
	{
		return MyVector(X() / vec.X(), Y() / vec.Y(), Z() / vec.Z());
	}

	static float length(MyVector vec)
	{
		return sqrt(vec.X() * vec.X() + vec.Y() * vec.Y() + vec.Z() * vec.Z());
	}

	static MyVector cross(MyVector vec1, MyVector vec2)
	{
		return MyVector(vec1.Y() * vec2.Z() - vec1.Z() * vec2.Y(), vec1.Z() * vec2.X() - vec1.X() * vec2.Z(), vec1.X() * vec2.Y() - vec1.Y() * vec2.X());
	}

	static MyVector normalize(MyVector vec)
	{
		return vec / MyVector::length(vec);
	}
};