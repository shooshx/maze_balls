#include "MazeBalls.h"
#include "MeshControl.h"
#include "MyLib/GLWidget.h"
#include "Mesh.h"

using namespace std;

MazeBalls::MazeBalls(QWidget *parent)
    : QMainWindow(parent)
{
    GLWidget::initFormat();
    ui.setupUi(this);

    m_meshHandler = new MeshControl(ui.glwidget, this);
    
    ui.glwidget->m_handlers.push_back(m_meshHandler);
    ui.glwidget->switchHandler(m_meshHandler);
}


void MazeBalls::on_runBut_clicked() 
{
    run();
}

void MazeBalls::run()
{
    cout << "running!" << endl;
    Mesh* m = new Mesh;
    m->loadObj("C:\\Projects\\maze_balls\\src\\icosahedron.obj");
    m->m_uniformColor = true;

    m_meshHandler->m_mesh.reset(m);
    ui.glwidget->setNewMinMax(m->m_pmin, m->m_pmax, true);

    ui.glwidget->update();
}