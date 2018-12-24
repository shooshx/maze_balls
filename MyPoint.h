#pragma once

#include "MyLib/general.h"
#include "MyLib/Vec.h"

class MyPoint;
class MyPolygon;

// for maze
class Edge
{
public:
    bool walled = true;
};


/** HalfEdge objects are used in the process of polygon subdivision in MyObject::subdivide().
	For every subdivision iteration performed, a complete half-edge model is generated
	for the polygon mesh. a half-edge is a tupple of an edge of a polygon and one of
	the vertices it touces.
	\see MyPolygon MyObject::subdivide()
*/
class HalfEdge
{
public:
	HalfEdge() 
    {}
	HalfEdge(MyPolygon *_poly, MyPoint *_pnt, HalfEdge *_next)
		:point(_pnt), pair(nullptr), poly(_poly), next(_next), edgePoint(nullptr) 
    {}
	void init(MyPolygon *_poly, MyPoint *_pnt, HalfEdge *_next)
	{ 
        point = _pnt; pair = nullptr; poly = _poly; next = _next; edgePoint = nullptr; 
    }

	MyPoint* point = nullptr; ///< point of origin of the half edge.
	HalfEdge* pair = nullptr; ///< matchin half-edge sibling
	MyPolygon* poly = nullptr; ///< the polygon this half edge is part of
	HalfEdge* next = nullptr; ///< the next half edge in the polygon circumference.

	/// the new point generated in this edge in the subdivision process
	/// this point is shared between the two edges which make the polygon edge.
	MyPoint* edgePoint = nullptr; 

    Edge* edge = nullptr;
};


class MyPoint  
{
public:
	MyPoint(float inX = 0.0, float inY = 0.0, float inZ = 0.0)
		:p(inX, inY, inZ), he(nullptr), touched(false) { ++g_ctorCount; }
	MyPoint(const MyPoint& a)
		:p(a.p), he(nullptr), touched(false) { ++g_ctorCount; }

	/// explicit conversion from Coord3df. we don't want to convert it to MyPoint 
	/// by accident
	explicit MyPoint(const Vec3& c) 
		:p(c), he(nullptr), touched(false) { ++g_ctorCount; }

	~MyPoint() { ++g_dtorCount; }
	
	void clear() { 
        p.clear(); 
        n.clear(); 
    }
	void setp(const Vec3 &c) { 
        const_cast<Vec3&>(p) = c; 
    }
	uint hash() const { 
        const uint *up = reinterpret_cast<const uint*>(p.ptr()); 
        return bXor(up[0], bXor(up[1] >> 8, up[2] >> 16));
    }

	MyPoint &operator+=(const MyPoint& a) { 
        p += a.p; 
        return *this; 
    }
	MyPoint &operator/=(float s) {
        p /= s; 
        return *this; 
    }
	MyPoint &operator*=(float s) { 
        p *= s; 
        return *this; 
    }
	MyPoint &operator=(const MyPoint& a) {
        p = a.p; 
        n.clear();
        touched = false;
        return *this; 
    }

	Vec3 p; // actual coordinate value
	Vec3 n; // normal of this point.
    Vec3 col = Vec3(0.7, 0.7, 0.7);

	// temporarys
	HalfEdge *he = nullptr; // HalfEdge originating from this point. - used in MyObject::subdivide()
    // was this point adjusted in the current subdiv iteration? - used in MyObject::subdivide()
    // in no-subdiv meshes, did this point have a set normal? or do we need to calc it (for flat faces)
	bool touched = false; 
             

	// managment
	static int g_ctorCount, g_dtorCount; // keep bookmarking of creation and deletion for debug
};

inline bool operator==(const MyPoint &p1, const MyPoint &p2) { return (p1.p == p2.p); }


enum ELineType {
    LINE_NORMAL
};

/** MyLine is a single line drawned using a specific color between two MyPoint instance.
	lines are used in the 3D solution display engine to mark the edges of pieces in 
	a clear manner. MyLine objects are not part of MyObject instances. they are drawn
	seperatly using LinesCollection.
	\see MyPoint
*/
class MyLine  
{
public:
	Vec3 color;
	MyPoint *p1, *p2; 

	ELineType type;

	MyLine(MyPoint *_p1, MyPoint *_p2, float clR, float clG, float clB, ELineType _type)
	: color(clR, clG, clB), p1(_p1), p2(_p2), type(_type)	{}
	MyLine() {} ///< empty ctor, needed for it to be possible to create arrays of MyLine
};
