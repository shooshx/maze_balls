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

    void setMesh(Mesh* m) {
        m_mesh.reset(m);
        m_bgl->setNewMinMax(m->m_pmin, m->m_pmax, true);
    }

public:

    FlatProgram m_progFlat; // for choice selection
   // Vec3 m_modelmin, m_modelmax;


    unique_ptr<Mesh> m_mesh;
    MazeBalls* m_doc;


protected:
    virtual void initialized() override;
    virtual void myPaintGL(bool inChoise) override;
    virtual void drawTargets(bool inChoise);

    virtual void scrPress(bool rightButton, int x, int y) override;
    virtual void scrRelease(bool rightButton, int x, int y) override;
    virtual bool scrMove(bool rightButton, bool ctrlPressed, int x, int y) override;
    virtual bool scrDblClick(bool ctrlPressed, int x, int y) override;


public:

};