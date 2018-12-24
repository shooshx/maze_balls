#pragma once

#include "MyPoint.h"
#include "MyLib/general.h"

class Texture;


class TexAnchor
{
public:
	TexAnchor(double _x = 0.0, double _y = 0.0) :x(_x), y(_y) {}

	float x, y; // the mapping on the texture
};



class MyPolygon  
{
public:

	MyPolygon(MyPoint* inVtx[], TexAnchor *inAncs)
	 : he(nullptr), f(nullptr)
	{	// point are shallow and anchors are deep copied
		init(inVtx, inAncs);
		++g_ctorCount;
	}

	MyPolygon() { ++g_ctorCount; } 
	~MyPolygon() { ++g_dtorCount; }

	// two versions, to help the branch prediction :)
	void init(MyPoint* inVtx[], TexAnchor *inAncs = nullptr)
	{
        for (int i = 0; i < 4; ++i)
            vtx[i] = inVtx[i];
        if (inAncs != nullptr)
            for (int i = 0; i < 4; ++i)
                texAncs[i] = inAncs[i];

        pnum = (inVtx[3] == nullptr) ? 3 : 4;
	}

	void init(TexAnchor *inAncs)
	{
        if (inAncs == nullptr)
            return;
		for(int i = 0; i < 4; ++i)
		{
			texAncs[i] = inAncs[i];
		}
	}

	/// calculate the normal of this polygon which is the vector pependicular to it.
	/// this is done with some math using the polygon's vertices.
	/// if none of the vertices for a polygon that is not a line an asserion is asserted.
	/// That assertion means there is a bug somewhere.
	void calcNorm();

    int pnum;
    MyPoint *vtx[4]; ///< A vector list of the polygons points in their order
	
	Vec3 center; ///< The normal of polygon

	TexAnchor texAncs[4]; ///< Texture anchors by the order of 0,0 - 1,0 - 1,1 - 0,1
	//Texture *tex; 
	// relevant only to half-textured pieces per-polygon texture, use sort for this to be efficient
	// nullptr to disable texturing

	HalfEdge *he = nullptr;	
	MyPoint *f = nullptr; // the center of polygon point.

	static int g_ctorCount, g_dtorCount; ///< keep bookmarking of creation and deletion for debug

    // maze:
    bool visited = false;
};


