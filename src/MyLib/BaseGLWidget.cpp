#include "glGlob.h"
#include "BaseGLWidget.h"
#include "ShaderProgram.h"

#include <sstream>
#include <iostream>

#include <string>
using namespace std;


using namespace std;

BaseGLWidget::BaseGLWidget()
    : m_minScaleReset(0), m_cxClient(1), m_cyClient(1), m_handler(nullptr)
{
    m_axis = XYaxis;
    m_cullFace = true;

    m_fUseLight = false;
    m_bSkewReset = false;

    m_zoomVal = 100;

    // some defaults to get things started
    aqmin = Vec3(-3, -3, -3);
    aqmax = Vec3(3, 3, 3);
    m_choiceBufferValid = false;
}

void BaseGLWidget::init()
{
//    ShaderProgram::shadersInit();

    checkErrors("init start");
    // specify black as clear color
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    // specify the back of the buffer as clear depth (0 closest, 1 farmost)
#ifdef EMSCRIPTEN
    glClearDepthf( 1.0f );
#else
    glClearDepth(1.0f);
    glEnable(0x809D);  //GL_MULTISAMPLE (not ES!)
#endif
    // enable depth testing (Enable zbuffer - hidden surface removal)
    glEnable(GL_DEPTH_TEST);
    if (m_cullFace)
       glEnable(GL_CULL_FACE);

//    glDepthFunc(GL_LEQUAL);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


    reset();
    checkErrors("init end");

    for(auto it = m_handlers.begin(); it != m_handlers.end(); ++it)
        (*it)->initialized();
}

void BaseGLWidget::switchHandler(GLHandler* handler) {
    if (m_handler)
        m_handler->switchOut();
    m_handler = handler;
    if (m_handler)
        m_handler->switchIn();
    reCalcProj();
}

// copied from mesa glu libutil/project.c
static void sgluMakeIdentityd(double m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}


#define glPi 3.14159265358979323846

static void sgluPerspective(double fovy, double aspect, double zNear, double zFar, float dest[16])
{
    double m[4][4];
    double sine, cotangent, deltaZ;
    double radians = fovy / 2 * glPi / 180;

    deltaZ = zFar - zNear;
    sine = sin(radians);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
        return;
    }
    cotangent = cos(radians) / sine;

    sgluMakeIdentityd(&m[0][0]);
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1;
    m[3][2] = -2 * zNear * zFar / deltaZ;
    m[3][3] = 0;
    //glMultMatrixd(&m[0][0]);
    for(int i = 0; i < 16; ++i) 
        dest[i] = (float)((&m[0][0])[i]);
}


void BaseGLWidget::reCalcProj() // = true default
{
    //cout << "WndSize=" << m_cxClient << "x" << m_cyClient << endl;

    //cout << "reCalc " << aqmin << "," << aqmax << endl;
    double figX = mMax(aqmax[0] - aqmin[0], (float)m_minScaleReset),
            figY = mMax(aqmax[1] - aqmin[1], (float)m_minScaleReset),
            figZ = mMax(aqmax[2] - aqmin[2], (float)m_minScaleReset);

    m_scrScale = mMin(mMin(4/figX, 4/figY), 4/figZ)*0.7;
    m_realScale = mMin(mMin(m_cxClient/figX, m_cyClient/figY), m_cxClient/figZ)*0.7; // for translate

    m_aspectRatio = (double)m_cxClient/(double)m_cyClient;
    //cout << "proj " << m_cxClient << "x" << m_cyClient << endl;

    glViewport(0, 0, m_cxClient, m_cyClient);
    
    //proj.cur().identity();

    //Mat4 p;
    sgluPerspective(60.0, m_aspectRatio, 1.0, 10.0, proj.cur().m);
    //proj.mult(p);
    proj.translate(0.0f, 0.0f, -4.0f);
    proj.scale(m_scrScale, m_scrScale, m_scrScale);
    
    sgluPerspective(60.0, 1.0, 1.0, 10.0, m_fixedAspectProj.m);
    m_fixedAspectProj.translate(0.0f, 0.0f, -4.0f);

}


Mat4 BaseGLWidget::transformMat() {
    return Mat4::multt(proj.cur(), model.cur());
}

Mat4 BaseGLWidget::getInitRotation() const {
    Mat4 m;
    m.identity();
    if (m_bSkewReset) {
        m.rotate(-20, 0, 1, 0);
        m.rotate(20, 1, 0, 0);
        m.rotate(-5, 0, 0, 1);
    }
    return m;
}

void BaseGLWidget::reset()
{
    m_zoomVal = 100;

    reCalcProj();
    model.set(getInitRotation());
} 

void BaseGLWidget::resize(int width, int height) {
    m_cxClient = width;
    m_cyClient = height;
    reCalcProj();
}

void BaseGLWidget::setNewMinMax(const Vec3& min, const Vec3& max, bool scale)
{
    //cout << "MINMAX " << min << "," << max << endl;
    aqmin = min; aqmax = max;
    if (scale) {
        reCalcProj();
    }

}

double BaseGLWidget::zoomFactor()
{
    double perc = m_zoomVal / 100.0;
    if (perc <= 1)
        return perc*perc;
    else
        return pow(3, perc) - 2;
}

void BaseGLWidget::modelMinMax(const Vec3& mqmin, const Vec3& mqmax)
{
    //cout << "MMM " << mqmin << " : " << mqmax << endl;
    model.translate(-(mqmax[0] + mqmin[0])/2, -(mqmax[1] + mqmin[1])/2, -(mqmax[2] + mqmin[2])/2);
}

void BaseGLWidget::paint(bool inChoise)
{
    checkErrors("cur");
//    model.push();
//    double zv = zoomFactor();
//    model.scale(zv, zv, zv);
//    model.translate(-(aqmax[0] + aqmin[0])/2, -(aqmax[1] + aqmin[1])/2, -(aqmax[2] + aqmin[2])/2);
    // a change here needs also a change in callDrawTargets!!
    if (m_handler)
        m_handler->myPaintGL(inChoise);
//    model.pop();
    checkErrors("paint");
}


/*
void BaseGLWidget::callDrawTargets()
{
    model.push();
    double zv = zoomFactor();
    //double zv = 2.1, m_zoomVal / 100.0;
    model.scale(zv, zv, zv);

    model.translate(-(aqmax[0] + aqmin[0])/2, -(aqmax[1] + aqmin[1])/2, -(aqmax[2] + aqmin[2])/2);

    if (m_handler)
        m_handler->drawTargets(true);	// the virtual function

    model.pop();
}
*/


static void sgluPickMatrix(double x, double y, double deltax, double deltay, int viewport[4], MatStack& mat)
{
    if (deltax <= 0 || deltay <= 0) { 
        return;
    }

    mat.translate((viewport[2] - 2 * (x - viewport[0])) / deltax, (viewport[3] - 2 * (y - viewport[1])) / deltay, 0);
    mat.scale(viewport[2] / deltax, viewport[3] / deltay, 1.0);
}


int BaseGLWidget::doChoise(int chX, int chY)
{
    static vector<int> buffer;
    static int bufWidth = 0, bufHeight = 0;
    if (bufWidth != m_cxClient || bufHeight != m_cyClient) {
        bufWidth = m_cxClient;
        bufHeight = m_cyClient;
#ifndef EMSCRIPTEN
        buffer.resize(bufWidth * bufHeight);
#else
        EM_ASM_(readFrameBuf = new Uint8Array($0); readFrameView = new DataView(readFrameBuf.buffer), bufWidth * bufHeight * 4);
#endif
    }

    if (chX < 0 || chY < 0 || chX >= bufWidth || chY >= bufHeight)
        return -1;


    if (!m_choiceBufferValid)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       // callDrawTargets();
        paint(true);
        
        //proj.pop();

        //glReadPixels(viewport[2] / 2, viewport[3] / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, buf);
#ifndef EMSCRIPTEN
        glReadPixels(0, 0, bufWidth, bufHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
#else
        EM_ASM_( GLctx.readPixels(0, 0, $0, $1, GLctx.RGBA, GLctx.UNSIGNED_BYTE, readFrameBuf), bufWidth, bufHeight);
#endif
        m_choiceBufferValid = true;
        m_screenNeedUpdate = true;
    }

    int index = (bufHeight - 1 - chY) * bufWidth + chX;
#ifndef EMSCRIPTEN
    int choose = buffer[index];
#else
    int choose = EM_ASM_INT(return readFrameView.getUint32($0, true), index * 4);
#endif
    choose &= 0xffffff;
    if (choose == 0)
        return -1;

    return choose;

}



void BaseGLWidget::rotate(EAxis axis, int x, int y) 
{
    Mat4 wmat;
    double fx = (double)x, fy = (double)y;

    wmat = model.cur();
    model.identity();

    switch (axis)
    {
    case Xaxis:	model.rotate(fx, 1, 0, 0); break;
    case Yaxis:	model.rotate(fx, 0, 1, 0); break;
    case Zaxis:	model.rotate(fx, 0, 0, 1); break;
    case XYaxis: model.rotate(fx, 0, 1, 0); model.rotate(fy, 1, 0, 0); break;
    case XZaxis: model.rotate(fx, 0, 0, 1); model.rotate(fy, 1, 0, 0); break;
    case YZaxis: model.rotate(fx, 0, 1, 0); model.rotate(fy, 0, 0, 1); break;
    }

    model.mult(wmat);

}

void BaseGLWidget::translate(int xValue, int yValue) 
{
    double fx = ((double)xValue / m_realScale);
    double fy = ((double)yValue / m_realScale);
    Mat4 wmat;

    wmat = model.cur();
    model.identity();

    switch (m_axis)
    {
    case Xaxis: model.translate(fx, 0, 0); break;
    case Yaxis: model.translate(0, fx, 0); break;
    case Zaxis: model.translate(0, 0, fx); break;
    case XYaxis: model.translate(fx, 0, 0); model.translate(0, -fy, 0); break;
    case XZaxis: model.translate(fx, 0, 0); model.translate(0, 0, fy); break;
    case YZaxis: model.translate(0, 0, fx); model.translate(0, -fy, 0); break;
    }

    model.mult(wmat);
}

const char* errorText(uint code)
{
    switch(code)
    {
    case GL_NO_ERROR: return "GL_NO_ERROR";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
#ifdef QT_CORE_LIB
    case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
#endif
    default: return "-Unknown Error Code-";
    }
}

#define CHECK_GL_ERR

string mglCheckErrorsStr(const char* place)
{
#ifdef CHECK_GL_ERR // slows down WebGL signifIcantly
    stringstream ss;
    GLenum code;

    //cout << "check " << place << endl;
    if (place != nullptr)
        ss << "GLError: " << place << "\n";
    int count = 0;
    while ((code = glGetError()) != GL_NO_ERROR && count++ < 10)
        ss << "0x" << hex << code << errorText(code) << endl;
    if (count != 0)
        return ss.str();
#endif
    return string();
}

void mglCheckErrors(const char* place) {
    string s = mglCheckErrorsStr(place);
    if (s.empty())
        return;

#if 0
    int ret = QMessageBox::critical(nullptr, "GLError", s, "Continue", "Exit", "Break");
    if (ret == 1)
        TerminateProcess(GetCurrentProcess(), 1);
    if (ret == 2)
        __asm int 3;
#else
    cout << s << endl;
#endif
}

void mglCheckErrors(const string& s) {
    mglCheckErrors(s.c_str());
}
void mglCheckErrorsC(const char* place)
{
    string s = mglCheckErrorsStr(place);
    if (s.empty())
        return;
    cout << s << endl;
 }
void mglCheckErrorsC(const string& s) {
    mglCheckErrorsC(s.c_str());
}

void BaseGLWidget::checkErrors(const char* place) {
    mglCheckErrorsC(place);
}


void BaseGLWidget::mousePress(int button, int x, int y)
{
    if (m_handler)
        m_handler->scrPress(button == MBUTTON_RIGHT, x, y);

    m_lastPos = Vec2i(x, y);
}

void BaseGLWidget::mouseRelease(int button, int x, int y)
{
    if (m_handler)
        m_handler->scrRelease(button != 0, x, y);
}

bool BaseGLWidget::mouseDoubleClick(bool hasCtrl, int x, int y)
{
    if (m_handler) {
        return m_handler->scrDblClick(hasCtrl, x, y);
    }
    return false;
}

void BaseGLWidget::mouseWheelEvent(int delta)
{
    m_zoomVal = mMin(mMax(m_zoomVal + delta, ZOOM_MIN), ZOOM_MAX);
    invalidateChoice();
}

bool BaseGLWidget::mouseMove(int buttons, int keyModify, int x, int y)
{
    bool needupdate = false;
    if (m_handler)
        needupdate = m_handler->scrMove(buttons != 0, (keyModify & KEYMOD_CTRL), x, y);
    if (buttons == 0) {
        return needupdate;
    }
    if (m_handler) // don't want to see selection while moving
        m_handler->clearChoise();

    int dx = x - m_lastPos.x;
    int dy = y - m_lastPos.y;

    if (m_handler) {
        if (m_handler->scrDrag(keyModify, dx, dy)) {
            m_lastPos = Vec2i(x, y);
            return true;
        }
    }

    auto axis = m_axis;
    if (keyModify & KEYMOD_SHIFT) { // shift
        axis = XZaxis;
    }
    switch (m_mouseAct)
    {			// act based on the current action
    case Rotate:
        rotate(axis, dx, dy);
        break;
    case Translate:
        translate(dx, dy);
        break;
        //case Scale:     
        //scale(dx, dy); 
        //    break;
    }
    invalidateChoice();
    m_lastPos = Vec2i(x, y);

    return true;

}

void BaseGLWidget::addProgressable(IProgressable* p) {
    m_progressables.push_back(p);
}

bool BaseGLWidget::progress(float deltaSec)
{
    bool needMore = false;
    auto it = m_progressables.begin();
    while (it != m_progressables.end())
    {
        bool b = (*it)->progress(deltaSec);
        needMore |= b;
        if (!b) 
            it = m_progressables.erase(it); // remove those who return false
        else
            ++it;
    }
    return needMore;
}

