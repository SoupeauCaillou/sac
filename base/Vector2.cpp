//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2008-2010, Shane J. M. Liesegang
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright 
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright 
//       notice, this list of conditions and the following disclaimer in the 
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the copyright holder nor the names of any 
//       contributors may be used to endorse or promote products derived from 
//       this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////////////

#include "Vector2.h"

#include "MathUtil.h"

#include <math.h>

Vector2 Vector2::Zero(0.0f, 0.0f);
Vector2 Vector2::One(1.0f, 1.0f);
Vector2 Vector2::UnitX(1.0f, 0.0f);
Vector2 Vector2::UnitY(0.0f, 1.0f);

Vector2::Vector2(float x, float y)
: X(x)
, Y(y)
{}

Vector2::Vector2(float value)
: X(value)
, Y(value)
{}

Vector2::Vector2()
: X(0)
, Y(0)
{}

float Vector2::Length() const
{
	return sqrt(LengthSquared());
}

float Vector2::LengthSquared() const
{
	return (X * X) + (Y * Y);
}

/*static*/ float Vector2::Distance(const Vector2& value1, const Vector2& value2)
{
	return Vector2(value1 - value2).Length();
}

/*static*/ float Vector2::DistanceSquared(const Vector2& value1, const Vector2& value2)
{
	return Vector2(value1 - value2).LengthSquared();

}

/*static*/ float Vector2::Dot(const Vector2& value1, const Vector2& value2)
{
	return ((value1.X * value2.X) + (value1.Y * value2.Y));
}

void Vector2::Normalize()
{
	float len = Length();


	if( len < 1e-7f )
	{
		if( Y > X )
			*this = UnitY;
		else
			*this = UnitX;
	}
	else
	{
		*this = *this / len;
	}
}

/*static*/ Vector2 Vector2::Normalize(const Vector2& value)
{
	Vector2 retVal(value);
	retVal.Normalize();
	return retVal;
}

/*static*/ Vector2 Vector2::Reflect(const Vector2& vector, const Vector2& normal)
{
	return vector - (normal * 2.0f * Dot(vector, normal));
}

/*static*/ Vector2 Vector2::Min(const Vector2& value1, const Vector2& value2)
{
	return Vector2(MathUtil::Min(value1.X, value2.X), MathUtil::Min(value1.Y, value2.Y));
}

/*static*/ Vector2 Vector2::Max(const Vector2& value1, const Vector2& value2)
{
	return Vector2(MathUtil::Max(value1.X, value2.X), MathUtil::Max(value1.Y, value2.Y));
}

/*static*/ Vector2 Vector2::Clamp(const Vector2& value, const Vector2& min, const Vector2& max)
{
	return Vector2(MathUtil::Clamp(value.X, min.X, max.X), MathUtil::Clamp(value.Y, min.Y, max.Y));
}

/*static*/ Vector2 Vector2::Lerp(const Vector2& value1, const Vector2& value2, float amount)
{
	return Vector2( MathUtil::Lerp( value1.X, value2.X, amount ), MathUtil::Lerp( value1.Y, value2.Y, amount ) );
}

Vector2 Vector2::Perp() const {
    return Vector2(-Y, X);
}

/*static*/ Vector2 Vector2::Negate(const Vector2& value)
{
	return -value;
}

/*static*/ Vector2 Vector2::Rotate(const Vector2& value, const float radians)
{
    float c = cos(radians);
    float s = sin(radians);
    return Vector2(value.X*c-value.Y*s,value.Y*c+value.X*s);
}

bool Vector2::operator==(const Vector2 &v) const
{
	return X == v.X && Y == v.Y;
}

bool Vector2::operator!=(const Vector2 &v) const
{
	return !(*this == v);
}

Vector2 Vector2::operator-() const
{
	return Vector2::Zero - *this;
}

Vector2 Vector2::operator-(const Vector2 &v) const
{
	return Vector2(X - v.X, Y - v.Y);
}

Vector2 Vector2::operator+(const Vector2 &v) const
{
	return Vector2(X + v.X, Y + v.Y);
}

Vector2 Vector2::operator/(float divider) const
{
	return Vector2(X / divider, Y / divider);
}

Vector2 Vector2::operator*(float scaleFactor) const
{
	return Vector2(X * scaleFactor, Y * scaleFactor);
}

Vector2& Vector2::operator+=(const Vector2 &v)
{
    X += v.X;
    Y += v.Y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2 &v)
{
    X -= v.X;
    Y -= v.Y;
    return *this;
}

Vector2& Vector2::operator*=(float scaleFactor)
{
    X *= scaleFactor;
    Y *= scaleFactor;
    return *this;
}

Vector2& Vector2::operator/=(float scaleFactor)
{
    X /= scaleFactor;
    Y /= scaleFactor;
    return *this;
}

Vector2 Vector2::VectorFromAngle(float radians) {
    return Vector2(cos(radians), sin(radians));
}



//TODO: when we actually make some unit tests, bring this back and put it there
// /*static*/ void Vector2::UnitTest()
// {
// 	Vector2 test(3, 4);
// 
// 	cout << test << endl;;
// 
// 	cout << "Length Sqr: " << test.LengthSquared() << endl;
// 
// 	cout << "Length: " << test.Length() << endl;
// 
// 
// 	cout << "Distance: " << Vector2::Distance(test, Vector2::Zero) << endl;
// 	cout << "Square Distance: " << Vector2::DistanceSquared(test, Vector2::Zero) << endl;
// 
// 	cout << "Dot: " << Vector2::Dot( Vector2(1), test ) << endl;
// 
// 	test.Normalize();
// 	cout << "Normal: " << test << " yields length: " << test.Length() << endl;
// 	test = Vector2(3,-4);
// 	{
// 		Vector2 checkIt(Vector2::Zero);
// 		cout << "Normal: " << Vector2::Normalize(checkIt) << endl;
// 
// 	}
// 
// 	cout << "Reflect: " << Vector2::Reflect( Vector2(10, -10), Vector2::Normalize( Vector2(-1, 1) )) << endl;
// 
// 	cout << "Max: " << Vector2::Max( Vector2(100), Vector2(9, 1000)) << endl;
// 	cout << "Min: " << Vector2::Min( Vector2(100), Vector2(9, 1000)) << endl;
// 	cout << "Clamp: " << Vector2::Clamp( Vector2(0), Vector2(9, 1000), Vector2(1000, 1100)) << endl;
// 
// 
// 	if( Vector2(10) == Vector2(10,10) )
// 		cout << "Equal!" << endl;
// 
// 	if( Vector2(0,1) != Vector2(1,0) )
// 		cout << "Not Equal!" << endl;
// 
// 	cout << (Vector2(10) + Vector2(9, 0)) << endl;
// 
// }
