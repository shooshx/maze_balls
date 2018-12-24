#include "MeshControl.h"
#include "MazeBalls.h"
#include <fstream>


#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif



void MeshControl::switchIn() {
    //m_bgl->m_minScaleReset = 10;

    reCalcSlvMinMax(); 
}

void MeshControl::reCalcSlvMinMax()
{

}


void MeshControl::initialized()
{
    glEnable(GL_MULTISAMPLE);
    m_progFlat.init();
    mglCheckErrorsC("progs");

    m_lightPos = Vec3(0.0, 20.0, 50.0);

    /*m_progNoise.init();
    {
        ProgramUser pu(&m_progNoise);
        m_progNoise.lightPos.set(Vec3(0.0f, 0.0f, 1000.0f));    
    }*/

    m_doc->run();
}



void MeshControl::paintBall(float zv, const MeshDisp& meshdisp, bool mirrorX)
{
    //m_bgl->modelMinMax(meshdisp.m->m_pmin, meshdisp.m->m_pmax);
    m_bgl->model.push();
    m_bgl->model.identity();
    float xzv = mirrorX ? -zv : zv;
    m_bgl->model.scale(xzv, zv, zv);
    m_bgl->model.translate(meshdisp.translate); // move it to the right side
    m_bgl->model.mult(m_bgl->model.peek(1)); // then do the rotation

    m_progFlat.setModelMat(m_bgl->model.cur());
    m_progFlat.trans.set(m_bgl->transformMat());
    m_progFlat.lightPos.set(m_lightPos);
    m_bgl->model.pop();

    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(1.0, 1.0);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    meshdisp.m->m_uColor = Vec3(1.0, 0.4, 0.4);
    meshdisp.m->paint(false, false);
}

void MeshControl::myPaintGL(bool inChoise)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_meshes.size() != 2) 
        return;


    double zv = m_bgl->zoomFactor();

    ProgramUser u(&m_progFlat);

    paintBall(zv, m_meshes[0], false);
    paintBall(zv, m_meshes[1], true);


   /* if (false) { // lines
        glPolygonOffset(0, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        m_mesh->m_uColor = Vec3(0.0, 0.0, 0.0);
        m_mesh->paint(inChoise, true);
    }*/

}

void MeshControl::drawTargets(bool inChoise)
{
}



bool MeshControl::scrDrag(bool ctrlPressed, int dx, int dy)
{
    if (!ctrlPressed)
        return false;
    Mat4 m;
    m.identity();
    m.rotate((float)dy, 1, 0, 0);
    m.rotate((float)dx, 0, 1, 0);
    m_lightPos = m.transformVec(m_lightPos);

    return true;
}



void MeshControl::saveBall(const MeshDisp& meshdisp, ofstream& f, int* vtx_offset, bool mirrorX)
{
    float zv = 1.0;
    m_bgl->model.push();
    m_bgl->model.identity();
    float xzv = mirrorX ? -zv : zv;
    m_bgl->model.scale(xzv, zv, zv);
    m_bgl->model.translate(meshdisp.translate); // move it to the right side
    m_bgl->model.mult(m_bgl->model.peek(1)); // then do the rotation

    Mesh mcopy = *meshdisp.m;
    mcopy.transform(m_bgl->model.cur());
    mcopy.save(f, vtx_offset, true);

    m_bgl->model.pop();
}

void MeshControl::save(const string& path) 
{
    ofstream f(path.c_str());
    if (!f.good()) {
        cout << "Failed opening path for mesh save " << path << endl;
        return;
    }
    int vtx_offset = 0;
    saveBall(m_meshes[0], f, &vtx_offset, false);
    saveBall(m_meshes[1], f, &vtx_offset, true);
}



