#include <iostream>

#ifndef _VECTORMATH_H_
#define _VECTORMATH_H_

typedef float real;

struct vector2
{
	real x, y;								//2 Dimensions
	vector2(const real& = 0.0f, const real& = 0.0f);			//Constructor
	vector2 normal();							//Normal
	void normalize();							//Normalize
	real magnitude();							//Magnitude
	vector2 operator +(const vector2&) const;				//Sum
	vector2 operator -(const vector2&) const;				//Difference
	real operator *(const vector2&) const;					//Dot Product
	vector2 operator *(const real&) const;					//Scale
	vector2 operator /(const real&) const;					//Scale
	void operator +=(const vector2&);					//Add
	void operator -=(const vector2&);					//Subtract
	void operator *=(const real&);						//Scale
	void operator /=(const real&);						//Scale
	std::istream &operator >>(std::istream&);
};

vector2 lerp(const vector2& = vector2(0.0, 0.0), const vector2& = vector2(0.0, 0.0), const real& = 0.0);
std::ostream &operator <<(std::ostream&, const vector2&);

struct vector3
{
	real x, y, z;								//3 Dimensions
	vector3(const real& = 0.0f, const real& = 0.0f, const real& = 0.0f);	//Constructor
	vector3(const vector3&);						//passed another vector3
	vector3 normal();							//Normal
	void normalize();							//Normalize
	real magnitude();							//Magnitude
	vector3 operator +(const vector3&) const;				//Sum
	vector3 operator -(const vector3&) const;				//Difference
	real operator *(const vector3&) const;					//Dot Product
	vector3 operator *(const real&) const;					//Scale
	vector3 operator /(const real&) const;					//Scale
	vector3 operator %(const vector3&) const;				//Cross Product
	void operator +=(const vector3&);					//Add
	void operator -=(const vector3&);					//Substract
	void operator *=(const real&);
	void operator /=(const real&);
	std::istream &operator >>(std::istream&);
};

vector3 operator* (const real& val1, const vector3& val2);
vector3 lerp(const vector3& = vector3(0.0, 0.0, 0.0), const vector3& = vector3(0.0, 0.0, 0.0), const real& = 0.0);
std::ostream &operator <<(std::ostream&, const vector3&);

struct vector4
{
	vector4(const real& = 0.0f, const real& = 0.0f, const real& = 0.0f, const real& = 0.0f);
	real x, y, z, w;

};

#endif
