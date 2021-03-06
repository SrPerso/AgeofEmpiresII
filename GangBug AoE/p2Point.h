// ----------------------------------------------------
// Point class    -----------
// ----------------------------------------------------

#ifndef __P2POINT_H__
#define __P2POINT_H__

#include "Defs.h"
#include <math.h>

template<class TYPE>
class p2Point
{
public:

	TYPE x, y;

	p2Point()
	{}

	p2Point(const p2Point<TYPE>& v)
	{
		this->x = v.x;
		this->y = v.y;
	}

	p2Point(const TYPE& x, const TYPE& y)
	{
		this->x = x;
		this->y = y;
	}

	p2Point& create(const TYPE& x, const TYPE& y)
	{
		this->x = x;
		this->y = y;

		return(*this);
	}

	// Math ------------------------------------------------
	p2Point operator -(const p2Point &v) const
	{
		p2Point r;

		r.x = x - v.x;
		r.y = y - v.y;

		return(r);
	}

	p2Point operator + (const p2Point &v) const
	{
		p2Point r;

		r.x = x + v.x;
		r.y = y + v.y;

		return(r);
	}
	p2Point operator *(const TYPE& f)
	{
		p2Point r;
		r.x = x * f;
		r.y = y * f;
		return (r);
	}

	p2Point operator /(const TYPE& f) //Care with 0 divisons.
	{
		p2Point r;
		r.x = x / f;
		r.y = y / f;
		return (r);
	}

	const p2Point& operator -=(const p2Point &v)
	{
		x -= v.x;
		y -= v.y;

		return(*this);
	}

	const p2Point& operator +=(const p2Point &v)
	{
		x += v.x;
		y += v.y;

		return(*this);
	}

	const p2Point& operator *=(const TYPE& f)
	{
		x *= f;
		y *= f;
		return (*this);
	}

	const p2Point& operator /=(const TYPE& f) //Care with 0 divisons.
	{
		x /= f;
		y /= f;
		return (*this);
	}

	bool operator ==(const p2Point& v) const
	{
		return (x == v.x && y == v.y);
	}

	bool operator !=(const p2Point& v) const
	{
		return (x != v.x || y != v.y);
	}

	// Utils ------------------------------------------------
	bool IsZero() const
	{
		return (x == 0 && y == 0);
	}

	p2Point& SetToZero()
	{
		x = y = 0;
		return(*this);
	}

	p2Point& Negate()
	{
		x = -x;
		y = -y;

		return(*this);
	}

	// Distances ---------------------------------------------
	TYPE DistanceTo(const p2Point& v) const
	{
		TYPE fx = x - v.x;
		TYPE fy = y - v.y;

		return sqrtf((fx*fx) + (fy*fy));
	}

	TYPE DistanceNoSqrt(const p2Point& v) const
	{
		TYPE fx = x - v.x;
		TYPE fy = y - v.y;

		return (fx*fx) + (fy*fy);
	}

	TYPE DistanceManhattan(const p2Point& v) const
	{
		return abs(v.x - x) + abs(v.y - y);
	}

	//Vector ---------------------------------------------------

	float GetModule()const
	{
		return DistanceTo({ 0, 0 });
	}

	void Normalize()
	{
		float module = GetModule();
		if (module != 0)
		{
			x /= module;
			y /= module;
		}
		else
		{
			x = y = 0;
		}
	}

	void Norm()
	{
		float len = sqrt(x*x + y*y);
		x /= len;
		y /= len;
	}

	const p2Point Normalized()const
	{
		p2Point p(x, y);
		p.Normalize();
		return p;
	}

	const p2Point GetNormal()const
	{
		float module = GetModule();
		if (module != 0)
			return ({ (x / module), (y / module) });
		else
			return ({ 0, 0 });
	}

	void SetModule(const float& f)
	{
		Normalize();

		x *= f;
		y *= f;
	}

	float GetAngle()const //Radians
	{
		return atan2(x, y);
	}

	float GetAngleDeg()const //Degrees
	{
		return atan2(x, y) * RADTODEG;
	}

	float GetDirectionAngle(p2Point& p)
	{
		return atan2((x - p.x), (y - p.y));
	}

	float GetDirectionAngleDeg(p2Point& p)
	{
		return atan2((x - p.x), (y - p.y)) * RADTODEG;
	}

	//---------------------------------------------------------

	const p2Point<float> ConvToFloat()
	{
		return p2Point<float>((float)x, (float)y);
	}

	const p2Point<int> ConvToInt()
	{
		return p2Point<int>((int)x, (int)y);
	}

	//-------------------------------------------------------------

};

typedef p2Point<int> iPoint;
typedef p2Point<float> fPoint;

#endif // __P2POINT_H__