#pragma once

#include "MyPolygon.h"
#include "MemoryMgmt.h"
#include <set>
#include <list>
#include <unordered_set>
using namespace std;



struct MyAllocator;
class Mesh;

/** MyObject is a mesh of polygons which make out a single piece in the 3D solution engine.
    The polygons and points are actually saved in an Indexed Face Set data structure.
    There is a main MyPoint repository and the polygons reference the points in there.
    MyObject instances are used only for creating the piece display list. after
    the list is created it is saved in the Piece's PicPainter member and the MyObject
    instance is discarded.
    \see PicPainter MyPolygon MyPoint MyLine
*/
class MyObject // should be called MyMesh
{
public:
    MyObject(MyAllocator* alloc = nullptr) 
        : m_alloc(alloc)
    {}
    
    vector<MyPolygon*> poly; // array sized nPoly of pointers to polygons
    vector<MyLine> lines;   // array sized nLines
    vector<MyPoint*> points; // array sized numPoint of pointers to the points

    int nPolys = 0, nLines = 0, nPoints = 0;


    MyPoint* copyPoint(const Vec3& c);
    void addPoly(Vec3 *inplst, TexAnchor *ancs = nullptr, Texture *tex = nullptr); //copy needed vertices, add poly
    void addPoly(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, bool flip = false, bool addNormal=false);
    void addLine(Vec3 *inp1, Vec3 *inp2, double inR, double inG, double inB, ELineType type);
    void vectorify();
    void clacNormals(bool vtxNormals);
    void clacNormalsExceptTouched();

    /// perform the subdivision algorithm over the current mesh, producing a mesh
    /// that has 4 times as many polygons.
    /// \arg smooth should the subdivision be averaged or flat (no averaging)
    void subdivide(bool smooth, bool clear_prev);

    void toMesh(Mesh& mesh, bool quads_to_tri, bool normals);

private:
    ///////////////////// structures for startup
    MyPoint* CopyCheckPoint(const Vec3 *p);

    list<MyPolygon*> plylst;
    list<MyLine> lnlst;
    list<MyPoint*> pntlst; // points can come either from the list or the map.

    MyAllocator *m_alloc; // used to allocate points, polygons and halfedges

    /// MyPointWrapper is a shallow wrapper for a MyPoint pointer.
    /// it is used for the comparison of points in order to avoid having to write 
    /// an operator=(MyPoint*, MyPoint*) which would be bad.
    struct MyPointWrapper 
    {
        explicit MyPointWrapper(MyPoint *p) :ptr(p) {}
        void detach(MyAllocator *m_alloc) const;
        mutable MyPoint *ptr; 
        // this is a workaround the constness of the set. we know the hash code is not
        // going to change due to the detach
    };

    struct MpHash {
        size_t operator()(const MyObject::MyPointWrapper &pnt) const {
            return pnt.ptr->hash();
        }
    };
    unordered_set<MyPointWrapper, MpHash> m_tmppoints; // used in startup to accumilate points. 

    friend bool operator==(const MyObject::MyPointWrapper &p1, const MyObject::MyPointWrapper &p2);

public:
    // add a point directly to the points repository, without duplicacy check.
    inline void basicAddPoint(MyPoint *pnt);
    // add a polygon made of points in the repository, without duplicacy checks.
    inline void basicAddPoly(MyPoint *inparr[], TexAnchor *ancs = nullptr);

    typedef vector<HalfEdge*> THalfEdgeList;

    THalfEdgeList m_lasthelst;
    bool buildHalfEdges(THalfEdgeList& lst);

};



