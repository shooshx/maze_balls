#include "MeshControl.h"
#include "MazeBalls.h"


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

    /*m_progNoise.init();
    {
        ProgramUser pu(&m_progNoise);
        m_progNoise.lightPos.set(Vec3(0.0f, 0.0f, 1000.0f));    
    }*/

    m_doc->run();
}

#ifdef EMSCRIPTEN
class JsGlTexture : public GlTexture
{
public:
    void registerBind(const char* imgname) {
        EM_ASM_( registerTexBind(Pointer_stringify($0), $1), imgname, m_obj);
    }
};
#endif

// called in the initialization
// preps the textures in the pieces bucket for the xml loading
void MeshControl::initTex()
{

}

void MeshControl::myPaintGL(bool inChoise)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_mesh) 
        return;


    double zv = m_bgl->zoomFactor();

    ProgramUser u(&m_progFlat);

    m_bgl->model.push();
    m_bgl->model.scale(zv, zv, zv);


    m_bgl->modelMinMax(m_mesh->m_pmin, m_mesh->m_pmax);
    m_progFlat.trans.set(m_bgl->transformMat());

    glEnable(GL_POLYGON_OFFSET_FILL);

    glPolygonOffset(1.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    m_mesh->m_uColor = Vec3(1.0, 0.4, 0.4);
    m_mesh->paint(inChoise);

    glPolygonOffset(0, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_mesh->m_uColor = Vec3(0.0, 0.0, 0.0);
    m_mesh->paint(inChoise);


    m_bgl->model.pop();
}



void MeshControl::drawTargets(bool inChoise)
{


}



void MeshControl::scrPress(bool rightButton, int x, int y)
{

}

void MeshControl::scrRelease(bool rightButton, int x, int y)
{

}

bool MeshControl::scrMove(bool rightButton, bool ctrlPressed, int x, int y)
{
    return false;
}

bool MeshControl::scrDblClick(bool hasCtrl, int x, int y)
{ 
    return false;
}
