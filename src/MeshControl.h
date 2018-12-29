#pragma once

#include "MyLib/BaseGLWidget.h"

#include "MyLib/Shaders.h"
#include "Mesh.h"
#include "MyLib/AutoParam.h"

class MazeBalls;

class MeshControl : public GLHandler
{
public:
    MeshControl(BaseGLWidget* gl, MazeBalls* doc)
        : GLHandler(gl), m_doc(doc)
    {}
    void initTex();

    void reCalcSlvMinMax();
    virtual void switchIn() override;

    void addMesh(Mesh* m, Vec3 translate) {
        //m_mesh.reset(m);
        m_meshes.push_back(MeshDisp{unique_ptr<Mesh>(m), translate});
        m_bgl->setNewMinMax(m->m_pmin, m->m_pmax, true);
    }
    void clearMeshes() {
        m_meshes.clear();
    }


public:

    FlatProgram m_progFlat; // for choice selection

    struct MeshDisp {
        unique_ptr<Mesh> m;
        Vec3 translate;
    };

    vector<MeshDisp> m_meshes;
    MazeBalls* m_doc;
    Vec3 m_lightPos;
    Mat4 m_globalTransform; // rotate the entire scene (not just one ball)

    Mesh m_planeMesh;


    void save(const string& path);
    void saveBall(const MeshDisp& meshdisp, ofstream& f, int* vtx_offset, bool mirrorX);


protected:
    virtual void initialized() override;
    virtual void myPaintGL(bool inChoise) override;


    virtual bool scrDrag(int keyModify, int dx, int dy) override;
    virtual bool scrDblClick(bool ctrlPressed, int x, int y) override;

    void paintBall(bool inChoise, float zv, const MeshDisp& meshdisp, bool mirrorX);
    void paintPlane();

    void transformForBall(float zv, bool mirrorX);

};