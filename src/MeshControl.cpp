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



void makePlane(Mesh& m, float szX, float szZ) {
    m.m_vtx.push_back(Vec3(szX, 0, szZ));
    m.m_vtx.push_back(Vec3(-szX, 0, szZ));
    m.m_vtx.push_back(Vec3(-szX, 0, -szZ));
    m.m_vtx.push_back(Vec3(szX, 0, -szZ));
    Vec3 n(0, 1, 0);
    m.m_normals.insert(m.m_normals.begin(), {n,n,n,n});
    m.m_idx.insert(m.m_idx.begin(), {0,1,2,3} );
    m.m_hasIdx = true;
    m.m_hasNormals = true;
    m.m_type = Mesh::QUADS;
    m.m_uColor = Vec3(1.0, 0.2, 0.2);
    m.m_uniformColor = true;
    m.makeSelfBos(false);
}


void MeshControl::initialized()
{
    glEnable(GL_MULTISAMPLE);
    m_progFlat.init();
    mglCheckErrorsC("progs");

    m_lightPos = Vec3(0.0, 20.0, 50.0);
    m_globalTransform.identity();

    makePlane(m_planeMesh, 2.1, 1.1);
    glDisable(GL_CULL_FACE);

    m_doc->run();
}



void MeshControl::paintBall(bool inChoise, float zv, const MeshDisp& meshdisp, bool mirrorX)
{
    //m_bgl->modelMinMax(meshdisp.m->m_pmin, meshdisp.m->m_pmax);
    m_bgl->model.push();
    m_bgl->model.identity();
    float xzv = mirrorX ? -zv : zv;

    //m_bgl->model.mult(m_globalTransform);
    m_bgl->model.scale(xzv, zv, zv);
    m_bgl->model.translate(meshdisp.translate); // move it to the right side
    m_bgl->model.mult(m_bgl->model.peek(1)); // then do the rotation

    //m_progFlat.setModelMat(m_bgl->model.cur());
    m_progFlat.modelMat.set(m_bgl->model.cur());
    // for the normals mat, we do want the global transform to participate since the light need to stay with the viewer
    m_progFlat.normalMat.set( Mat4::multt(m_globalTransform, m_bgl->model.cur()).toNormalsTrans());

    m_progFlat.projMat.set(m_bgl->proj.cur());
    //m_progFlat.trans.set(m_bgl->transformMat());
    m_bgl->model.pop();

    m_progFlat.lightPos.set(m_lightPos);
    m_progFlat.globalT.set(m_globalTransform);

    if (!inChoise)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        meshdisp.m->m_uColor = Vec3(1.0, 0.4, 0.4);
        meshdisp.m->paint(false, false);

        if (m_doc->m_params.wireframe) {
            glLineWidth(3);
            glPolygonOffset(0, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            meshdisp.m->m_uColor = Vec3(1.0, 0.0, 0.0);
            meshdisp.m->paint(false, true);

            glPolygonOffset(1.0, 1.0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    else
    { // named point for each vertex
        glPointSize(8);
        auto typbk = meshdisp.m->m_type;
        meshdisp.m->m_type = Mesh::POINTS;
        meshdisp.m->paint(true, false);
        meshdisp.m->m_type = typbk;
    }

}


void MeshControl::paintPlane()
{
    // m_globalTransform is already there
    m_progFlat.setModelMat(Mat4::ident());
    m_planeMesh.paint(false, true);
}


void MeshControl::myPaintGL(bool inChoise)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_STENCIL_BUFFER_BIT);

    if (m_meshes.size() != 2) 
        return;


    double zv = m_bgl->zoomFactor();

    ProgramUser u(&m_progFlat);

    if (m_doc->m_params.cross_section)
    {
        m_progFlat.b_clip_top.set(true);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 1);
        glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

        paintBall(inChoise, zv, m_meshes[0], false);
        paintBall(inChoise, zv, m_meshes[1], true);

        glStencilFunc(GL_EQUAL, 1, 1);
        glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);
        
        if (!inChoise)
            paintPlane();

        glDisable(GL_STENCIL_TEST);
    }
    else 
    {
        m_progFlat.b_clip_top.set(false);
        paintBall(inChoise, zv, m_meshes[0], false);
        paintBall(inChoise, zv, m_meshes[1], true);
    }

    
}



bool MeshControl::scrDblClick(bool ctrlPressed, int x, int y)
{
    int ch = m_bgl->doChoise(x, y);
    cout << "choice=" << ch + 1<< endl;
    return true;
}


bool MeshControl::scrDrag(int keyModify, int dx, int dy)
{
    if (keyModify & KEYMOD_CTRL)
    {
        Mat4 m;
        m.identity();
        m.rotate((float)dy, 1, 0, 0);
        m.rotate((float)dx, 0, 1, 0);
        m_lightPos = m.transformVec(m_lightPos);
        return true;
    }
    if (keyModify & KEYMOD_ALT)
    {
        Mat4 wmat = m_globalTransform;
        m_globalTransform.identity();

        m_globalTransform.rotate(dx, 0, 1, 0); 
        m_globalTransform.rotate(dy, 1, 0, 0);

        m_globalTransform.mult(wmat);
        m_bgl->invalidateChoice();
        return true;
    }

    return false;
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
    mcopy.save(f, vtx_offset, m_doc->m_params.save_quads, false);

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



