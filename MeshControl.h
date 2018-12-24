#pragma once

#include "MyLib/BaseGLWidget.h"

#include "MyLib/Shaders.h"
#include "Mesh.h"

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
   // Vec3 m_modelmin, m_modelmax;

    struct MeshDisp {
        unique_ptr<Mesh> m;
        Vec3 translate;
    };

    vector<MeshDisp> m_meshes;
    MazeBalls* m_doc;
    Vec3 m_lightPos;


    void save(const string& path);
    void saveBall(const MeshDisp& meshdisp, ofstream& f, int* vtx_offset, bool mirrorX);


protected:
    virtual void initialized() override;
    virtual void myPaintGL(bool inChoise) override;
    virtual void drawTargets(bool inChoise);

    virtual bool scrDrag(bool ctrlPressed, int dx, int dy) override;

    void paintBall(float zv, const MeshDisp& meshdisp, bool mirrorX);

    void transformForBall(float zv, bool mirrorX);

};