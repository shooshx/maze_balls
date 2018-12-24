// Happy Cube Solver - Building and designing models for the Happy cube puzzles
// Copyright (C) 1995-2006 Shy Shalom, shooshX@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "MyObject.h"
#include "Mesh.h"
#include <unordered_map>
using namespace std;


int MyPoint::g_ctorCount = 0, MyPoint::g_dtorCount = 0;
int MyPolygon::g_ctorCount = 0, MyPolygon::g_dtorCount = 0;

void MyObject::MyPointWrapper::detach(MyAllocator *m_alloc) const
{ 
    MyPoint *old = ptr;
    if (m_alloc != nullptr)
        ptr = m_alloc->m_pointsPool.allocate();
    else
        ptr = new MyPoint; // TBD - happens with lines. remove when lines go display list.
    *ptr = *old;
}

// checks if a point is in m_tmppoints, if not, copy and insert it
// returns the pointer to the permanent point
MyPoint* MyObject::CopyCheckPoint(const Vec3 *c)
{
    static MyPoint p;
    p.setp(*c);
    const MyPointWrapper &realpntw = *m_tmppoints.insert(MyPointWrapper(&p)).first;
    if (realpntw.ptr == &p) // it was inserted
        realpntw.detach(m_alloc);
    return realpntw.ptr;
}

// when there's no need to check duplication
MyPoint* MyObject::copyPoint(const Vec3& c)
{
    static MyPoint p;
    p.setp(c);
    MyPointWrapper realpntw(&p);
    realpntw.detach(m_alloc);
    basicAddPoint(realpntw.ptr);
    return realpntw.ptr;
}


void MyObject::addLine(Vec3 *inp1, Vec3 *inp2, double inR, double inG, double inB, ELineType type)
{	
    MyLine pln(nullptr, nullptr, inR, inG, inB, type);

    pln.p1 = CopyCheckPoint(inp1);
    pln.p2 = CopyCheckPoint(inp2);

    lnlst.push_back(pln);
}

// copies the points in the points array
void MyObject::addPoly(Vec3 *inplst, TexAnchor *ancs, Texture *tex)
{
    MyPolygon *nply = m_alloc->m_polyPool.allocate();
    nply->init(ancs);
    for (int lstp = 0; lstp < 4; ++lstp)
    {
        nply->vtx[lstp] = CopyCheckPoint(&inplst[lstp]);
    }

    plylst.push_back(nply);
}

// addNormal - calcs the normal of the poly and adds it to the points.
//   this is given to selected quads which need to retain their normals and not linearize it (piece face)
void MyObject::addPoly(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, bool flip, bool addNormal)
{
    MyPolygon *nply = m_alloc->m_polyPool.allocate();
    if (flip) {
        nply->vtx[3] = CopyCheckPoint(&a);
        nply->vtx[2] = CopyCheckPoint(&b);
        nply->vtx[1] = CopyCheckPoint(&c);
        nply->vtx[0] = CopyCheckPoint(&d);
    }
    else {
        nply->vtx[0] = CopyCheckPoint(&a);
        nply->vtx[1] = CopyCheckPoint(&b);
        nply->vtx[2] = CopyCheckPoint(&c);
        nply->vtx[3] = CopyCheckPoint(&d);
    }

    if (addNormal) { // add normals to these points according to this quad so that the normal would not blend
        nply->calcNorm();
        for(int i = 0; i < 4; ++i) {
            nply->vtx[i]->n = nply->center;
            nply->vtx[i]->touched = true;
        }
    }

    plylst.push_back(nply);
}

inline void MyObject::basicAddPoint(MyPoint *pnt) 
{ 
    pntlst.push_back(pnt); 
}

// add a polygon made of points in the repository, without duplicacy checks.
inline void MyObject::basicAddPoly(MyPoint *inparr[], TexAnchor *ancs)
{ 
    MyPolygon *nply = m_alloc->m_polyPool.allocate();
    nply->init(inparr, ancs);
    plylst.push_back(nply); 
}

bool operator==(const MyObject::MyPointWrapper &p1, const MyObject::MyPointWrapper &p2)
{
    return (*p1.ptr == *p2.ptr); // compare the actual values TBD..
}


void MyObject::vectorify()
{
    int insPoly = 0, insLines = 0, insPoint = 0;
    // points
    nPoints = m_tmppoints.size() + pntlst.size();
    points.resize(nPoints);

    for (auto pit = m_tmppoints.begin(); pit != m_tmppoints.end(); ++pit) {
        points[insPoint++] = const_cast<MyPoint*>(pit->ptr); // const_iterator is too restrictive
    }
    m_tmppoints.clear();
    for (auto plit = pntlst.cbegin(); plit != pntlst.cend(); ++plit)
    {
        points[insPoint++] = *plit;
    }
    pntlst.clear();

    nPolys = plylst.size();
    poly.resize(nPolys);

    // add the polygons sorted according to their texture, nullptr first
    for (auto lit = plylst.begin(); lit != plylst.end() ; ++lit)
    {
        poly[insPoly++] = *lit;
    }
    
    plylst.clear();

    nLines = lnlst.size();
    lines.resize(nLines);
    for (auto nit = lnlst.begin(); nit != lnlst.end(); ++nit)
    {
        lines[insLines++] = *nit;
    }
    lnlst.clear();

}

void MyPolygon::calcNorm() //use the regular point and not the temp one
{
	// assume all points are on the same plane;
	bool normAgain = true;
	int pntA = 0, pntB = 1, pntC = 2;
	double nx, ny, nz;
	double xa, ya, za, xb, yb, zb, xc, yc, zc;

	while (normAgain)
	{	

		xa = vtx[pntA]->p[0], ya = vtx[pntA]->p[1], za = vtx[pntA]->p[2];
		xb = vtx[pntB]->p[0], yb = vtx[pntB]->p[1], zb = vtx[pntB]->p[2];
		xc = vtx[pntC]->p[0], yc = vtx[pntC]->p[1], zc = vtx[pntC]->p[2];
	
		
		nx = (ya-yb)*(za-zc)-(ya-yc)*(za-zb);
		ny = -((xa-xb)*(za-zc)-(xa-xc)*(za-zb));
		nz = (xa-xb)*(ya-yc)-(xa-xc)*(ya-yb);
		
		normAgain = ((fabs(nx) < 0.00000001) && (fabs(ny) < 0.00000001) && (fabs(nz) <0.00000001));
		if (normAgain)
		{
			if (pntC < 3)
			{
				pntC++;
			}
			else
			{
				if (pntB < 2)
				{
					pntB++;
					pntC = pntB + 1;
				}
				else
				{
					if (pntA < 1)
					{
						pntA++;
						pntB = pntA + 1;
						pntC = pntA + 2;
					}
					else
					{
                        M_ASSERT(false);
					}
				}
			}
		}
        M_ASSERT((pntB < 4) && (pntC < 4));
		
	}
	double lng = sqrt(nx*nx + ny*ny + nz*nz);


	center[0] = nx / lng;
	center[1] = ny / lng;
	center[2] = nz / lng;

}


void MyObject::clacNormals(bool vtxNorms)
{
    for(int pn = 0; pn < nPoints; ++pn)
        points[pn]->n.clear();

    for (int i = 0; i < nPolys; ++i)
    {
        poly[i]->calcNorm();
        if (vtxNorms)
        {
            for(int pn = 0; pn < 4; ++pn)
                poly[i]->vtx[pn]->n += poly[i]->center;
        }
    }

    if (vtxNorms)
    {
        for (int pn = 0; pn < nPoints; ++pn)
            points[pn]->n.unitize();
    }

}

void MyObject::clacNormalsExceptTouched()
{
    for (int pn = 0; pn < nPoints; ++pn) {
        if (points[pn]->n.x != 0.0 || points[pn]->n.y != 0.0 || points[pn]->n.z != 0.0) {
            int x = 0;
        }
        //points[pn]->n.clear();
    }

    for (int i = 0; i < nPolys; ++i)
    {
        poly[i]->calcNorm();
        for(int pn = 0; pn < 4; ++pn) 
        {
            auto* p = poly[i]->vtx[pn];
            if (!p->touched)
                p->n += poly[i]->center;
        }
    }

    for (int pn = 0; pn < nPoints; ++pn) {
        points[pn]->n.unitize();
        points[pn]->touched = false;
    }

}


/// PointPair is a simple struct of two MyPoints objects.
/// it is used for hashing HalfEdge objects in MyObject::buildHalfEdges()
struct PointPair
{
    PointPair(MyPoint *_p1, MyPoint *_p2) :p1(_p1), p2(_p2) {}
    // p1 and p2 are interchangable
    MyPoint *p1;
    MyPoint *p2;
};

struct PpHash {
    size_t operator()(const PointPair& pp) const
    {
        uint x = (pp.p1->hash() + pp.p2->hash()); // commutative
    //	qDebug("%f,%f,%f:%f,%f,%f - %X", (*pp.p1).p[0], (*pp.p1).p[1], (*pp.p1).p[2], (*pp.p2).p[0], (*pp.p2).p[1], (*pp.p2).p[2], x);
        return x;
    }
};

bool operator==(const PointPair& pp1, const PointPair& pp2)
{	// compare pointers
    return ((pp1.p1 == pp2.p1) && (pp1.p2 == pp2.p2)) || ((pp1.p2 == pp2.p1) && (pp1.p1 == pp2.p2)); // commutative
}
typedef unordered_map<PointPair, HalfEdge*, PpHash> TPointPairHash;


bool MyObject::buildHalfEdges(THalfEdgeList& lst)
{
    int pli, i;

    TPointPairHash m_pntmap;
    m_pntmap.reserve(150);
    // the pointer is the half edge already between these points

    for(pli = 0; pli < nPolys; ++pli)
    {
        MyPolygon *pol = poly[pli];

        HalfEdge *he = nullptr, *lasthe = nullptr;
        for(i = pol->pnum - 1; i >= 0; --i)
        {
            //he = new HalfEdge(pol, pol->vtx[i], he);
            HalfEdge *prevhe = he;
            he = m_alloc->m_hePool.allocate();
            he->init(pol, pol->vtx[i], prevhe);
            lst.push_back(he);
            if (lasthe == nullptr)
                lasthe = he;

            if (pol->vtx[i]->he == nullptr) // first HalfEdge of this point
                pol->vtx[i]->he = he;

        }
        lasthe->next = he;
        pol->he = he;

        for(i = pol->pnum - 1; i >= 0; --i)
        {	
            PointPair pp(he->point, he->next->point);
            TPointPairHash::iterator it = m_pntmap.find(pp);
            if (it == m_pntmap.end()) // its not there, add it
                m_pntmap.insert(make_pair(pp, he));
            else
            {
                HalfEdge *she = it->second;
                M_ASSERT((she->next->point == he->point) && (he->next->point == she->point));
                he->pair = she;
                she->pair = he;
                m_pntmap.erase(it); // no longer needed in the map
            }
            he = he->next;
        }
        // maximum size of m_pntmap is 134
    }

    M_ASSERT(m_pntmap.empty());

    return true;
}

extern "C" unsigned long __stdcall GetTickCount();


void MyObject::subdivide(bool smooth, bool clear_prev)
{
    if (clear_prev) {
        m_alloc->m_hePool.clear();
        m_lasthelst.clear();
    }
    auto startTime = GetTickCount();
    //m_lasthelst.reserve(5000);
    m_lasthelst.reserve(nPolys * 4 + 100);
    buildHalfEdges(m_lasthelst);

    cout << "start poly=" << nPolys << "  points=" << nPoints << "  he=" << m_lasthelst.size() << endl;

    // face points
    for(int pli = 0; pli < nPolys; ++pli)
    {
        MyPolygon &pol = *poly[pli];
        pol.f = m_alloc->m_pointsPool.allocate();
        pol.f->clear();
        for(int i = 0; i < pol.pnum; ++i)
        {
            *pol.f += *pol.vtx[i];
            pol.vtx[i]->touched = false; // while we're at it, reset the point for the current iteration
        }
        *pol.f /= (float)pol.pnum;
        basicAddPoint(pol.f);
    }
    
    // edge points
    for(int i = 0; i < m_lasthelst.size(); ++i)
    {
        HalfEdge *he = m_lasthelst[i];
        if (he->edgePoint != nullptr) 
            continue;

        he->edgePoint = m_alloc->m_pointsPool.allocate();
        (*he->edgePoint) = *he->point;
        (*he->edgePoint) += *he->pair->point;
        if (smooth)
        {
            (*he->edgePoint) += *he->poly->f;
            (*he->edgePoint) += *he->pair->poly->f;
            (*he->edgePoint) /= 4.0;
        }
        else
            (*he->edgePoint) /= 2.0;

        he->pair->edgePoint = he->edgePoint;
        basicAddPoint(he->edgePoint);
    }

    for(int pli = 0; pli < nPolys; ++pli)
    {
        MyPolygon &pol = *poly[pli];
        MyPoint *e[4] = {0};

        int i = 0;
        HalfEdge *he = pol.he;
        do {
            e[i++] = he->edgePoint;
            he = he->next;
        } while (he != pol.he);

        
        if (smooth)
        {
            for(int i = 0; i < pol.pnum; ++i)
            {
                if (pol.vtx[i]->touched)
                    continue;

                Vec3 F, E; // average of all Face points near of op[i]

                double n = 0;
                he = pol.vtx[i]->he;
                do {
                    F += he->poly->f->p;
                    if (he->point == pol.vtx[i]) // pointer comparison
                        E += he->pair->point->p;
                    else
                        E += he->point->p;

                    he = he->pair->next;
                    n += 1.0;
                } while (he != pol.vtx[i]->he);

            /*	F /= n;
                E += *poly.vtx[i] * 3.0;
                E /= n * 2.0;
                op[i] = (F + (2.0 * E) + ((n - 3.0)*(*poly.vtx[i])))/n;*/

                // store the result intermediatly in 'n' so we wont affect the on-going calculation process
                double nsqr = n*n;
                pol.vtx[i]->n = (((n - 2.0)/n)*(pol.vtx[i]->p)) + (F/(nsqr)) + (E/(nsqr));

                pol.vtx[i]->he = nullptr; // (DON'T, we'll need it in the dfs) reset the half edge for the next time (and for the halfedge impending death)
                // this clearing is needed since setting the first HE checks if it's null
                pol.vtx[i]->touched = true;
                basicAddPoint(pol.vtx[i]); // add it again becuse the array is going to be deleted
            }
        }
        else
        {
            for(int i = 0; i < pol.pnum; ++i)
            {
                pol.vtx[i]->he = nullptr; // (DON'T) reset the half edge for the next time (and for the halfedge impending death)
                pol.vtx[i]->touched = true;
                basicAddPoint(pol.vtx[i]); // add it again becuse the array is going to be deleted
            }
        }

        //pol.he = nullptr; //(DON'T)
        MyPoint *laste = e[pol.pnum - 1];

        MyPoint *pl1[] = { pol.vtx[0], e[0], pol.f, laste };
        MyPoint *pl2[] = { e[0], pol.vtx[1], e[1], pol.f };
        MyPoint *pl3[] = { pol.f, e[1], pol.vtx[2], e[2] };
        MyPoint *pl4[] = { e[3], pol.f, e[2], pol.vtx[3] };


        TexAnchor tv[] = { pol.texAncs[0], pol.texAncs[1], pol.texAncs[2], pol.texAncs[3] }; // original texture anchors
        TexAnchor tf((tv[0].x + tv[2].x) / 2.0, (tv[0].y + tv[2].y) / 2.0);
        TexAnchor te[] = { TexAnchor((tv[0].x + tv[1].x) / 2.0, (tv[0].y + tv[1].y) / 2.0), 
                           TexAnchor((tv[1].x + tv[2].x) / 2.0, (tv[1].y + tv[2].y) / 2.0),
                           TexAnchor((tv[2].x + tv[3].x) / 2.0, (tv[2].y + tv[3].y) / 2.0),
                           TexAnchor((tv[3].x + tv[0].x) / 2.0, (tv[3].y + tv[0].y) / 2.0) };

		TexAnchor lastte = te[pol.pnum - 1];
        TexAnchor tx1[] = { tv[0], te[0], tf, lastte };
        TexAnchor tx2[] = { te[0], tv[1], te[1], tf };
        TexAnchor tx3[] = { tf, te[1], tv[2], te[2] };
        TexAnchor tx4[] = { te[3], tf, te[2], tv[3] };

        basicAddPoly(pl1, tx1);
        basicAddPoly(pl2, tx2);
        basicAddPoly(pl3, tx3);
        if (pol.pnum == 4)
            basicAddPoly(pl4, tx4);

    }

    if (smooth)
    {
        // go over all the points the the points array of the object. these are the original points
        for (int i = 0; i < nPoints; ++i) {
            points[i]->p = points[i]->n;
            points[i]->touched = false;
        }
    }

    vectorify();
    cout << "   subdivided poly=" << nPolys << "  points=" << nPoints << "  time=" << (GetTickCount() - startTime) << " msec" << endl;
}



void MyObject::toMesh(Mesh& mesh, bool quads_to_tri, bool normals, bool poly_reverse) 
{
    mesh.clear();
    if (nPolys == 0)
        return;
    VecRep vtxrep(&mesh.m_vtx);

    for(int pli = 0; pli < nPolys; ++pli) //polygons
    {
        MyPolygon &curpl = *poly[pli];
        int qidx[4];
        for(int pni = 0; pni < curpl.pnum; ++pni) //points
        {
            MyPoint *curpn = curpl.vtx[pni];
            int index = 0;
            if (vtxrep.add(curpn->p, &index)) {
                if (normals) {
                    mesh.m_normals.push_back(curpn->n);
                }
                //mesh.m_texCoord.push_back(Vec2(curpl.texAncs[pni].x, curpl.texAncs[pni].y));
                mesh.m_color4.push_back(Vec4(curpn->col.r, curpn->col.g, curpn->col.b, 1.0));
            }
            qidx[pni] = index;
            //if (quads)
            //mesh.m_idx.push_back(index);
        }

        if (!poly_reverse) {
            mesh.m_idx.push_back(qidx[0]);
            mesh.m_idx.push_back(qidx[1]);
            mesh.m_idx.push_back(qidx[2]);
            mesh.m_idx.push_back(qidx[3]);
        }
        else {
            mesh.m_idx.push_back(qidx[3]);
            mesh.m_idx.push_back(qidx[2]);
            mesh.m_idx.push_back(qidx[1]);
            mesh.m_idx.push_back(qidx[0]);
        }

        /*if (!quads) {
            mesh.m_idx.push_back(qidx[0]);
            mesh.m_idx.push_back(qidx[1]);
            mesh.m_idx.push_back(qidx[2]);
            mesh.m_idx.push_back(qidx[0]);
            mesh.m_idx.push_back(qidx[2]);
            mesh.m_idx.push_back(qidx[3]);
        }*/
    }

    bool isQuads = poly[0]->pnum == 4;

    mesh.m_type = isQuads ? Mesh::QUADS : Mesh::TRIANGLES;
    mesh.m_hasNormals = normals;
    mesh.m_hasColors = true;
    mesh.m_hasIdx = true;
    mesh.calcMinMax();
    mesh.makeSelfBos(false);
}