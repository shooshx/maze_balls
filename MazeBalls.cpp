#include "MazeBalls.h"
#include "MeshControl.h"
#include "MyLib/GLWidget.h"
#include "Mesh.h"
#include "MyObject.h"
#include <thread>

using namespace std;

MazeBalls::MazeBalls(QWidget *parent)
    : QMainWindow(parent)
{
    GLWidget::initFormat();
    ui.setupUi(this);

    m_meshHandler = new MeshControl(ui.glwidget, this);
    
    ui.glwidget->m_handlers.push_back(m_meshHandler);
    ui.glwidget->switchHandler(m_meshHandler);

    connect(this, &MazeBalls::update_mesh, this, &MazeBalls::call_update_mesh);

}


void MazeBalls::on_runBut_clicked() 
{
    run();
}


void meshToObj(const Mesh& mesh, MyObject& obj)
{
    vector<MyPoint*> pnt_ptrs(mesh.m_vtx.size());
    int i = 0;
    for(const auto& vtx: mesh.m_vtx)
        pnt_ptrs[i++] = obj.copyPoint(vtx);

    if (mesh.m_type == Mesh::TRIANGLES) {
        for(int i = 0; i < mesh.m_idx.size(); i += 3) {
            MyPoint* tri[4] = { pnt_ptrs[mesh.m_idx[i]], pnt_ptrs[mesh.m_idx[i+1]], pnt_ptrs[mesh.m_idx[i+2]], nullptr };
            obj.basicAddPoly(tri);
        }
    }
    else {
        for (int i = 0; i < mesh.m_idx.size(); i += 4) {
            MyPoint* tri[4] = { pnt_ptrs[mesh.m_idx[i]], pnt_ptrs[mesh.m_idx[i + 1]], pnt_ptrs[mesh.m_idx[i + 2]], pnt_ptrs[mesh.m_idx[i + 3]] };
            obj.basicAddPoly(tri);
        }
    }
    obj.vectorify();
}


struct Graph {
    
    vector<Edge> edges;

};

void createGraph(MyObject& obj, MyObject::THalfEdgeList& helst, Graph& graph)
{
    // we go over the half edges from the last subdivision, since that one has a half
    // edge for polys in the maze mesh

    // set middle edge for all half edges
    graph.edges.reserve(helst.size() / 2);
    for(auto he: helst) {
        if (he->pair->edge)
            he->edge = he->pair->edge;
        else {
            graph.edges.push_back(Edge());
            he->edge = &graph.edges[graph.edges.size() - 1];
        }
    }
}

std::function<void(void)> g_update_emit;
std::function<void(void)> g_update_call;

void recDfs(MyPolygon* p) 
{
    p->visited = true;
    p->f->col = Vec3(1.0, 0.0, 0.0);

    //g_update_emit();

    HalfEdge* he = p->he;
    for(int i = 0; i < 4; ++i) {
        he = he->next;
        if (!he->pair->poly->visited) {
            he->edge->walled = false;
            he->edgePoint->col = Vec3(1.0, 0.0, 0.0);
            recDfs(he->pair->poly);
        }
    }
    M_ASSERT(he == p->he); // sanity - reached around
}

void mazeDfs(MyObject& obj) 
{
    recDfs(obj.poly[0]);

}

void MazeBalls::call_update_mesh() {
    g_update_call();
}

void MazeBalls::run_thread()
{
    cout << "running!" << endl;
    unique_ptr<Mesh> load_mesh(new Mesh);
    const char* filename = "C:\\Projects\\maze_balls\\src\\icosahedron.obj";
    //const char* filename = "C:\\Projects\\maze_balls\\src\\cube.obj";
    load_mesh->loadObj(filename);
    load_mesh->m_uniformColor = true;

    MyAllocator obj_alloc;
    MyObject maze_obj(&obj_alloc); // the obj on which we DFS the maze

    meshToObj(*load_mesh, maze_obj);
    maze_obj.subdivide(true, true);
    maze_obj.subdivide(true, true);

    MyObject ridges_obj = maze_obj; // the obj that has the vallies and ridges
    ridges_obj.subdivide(true, false);

    Mesh* w_mesh = new Mesh;
    w_mesh->m_uniformColor = true;
    //maze_obj.toMesh(*w_mesh, false, false);
    ridges_obj.toMesh(*w_mesh, false, false);

    m_meshHandler->setMesh(w_mesh);
    ui.glwidget->update();


    g_update_emit = [&]() {
        emit_update_mesh();
        cout << "step" << endl;
        getchar();
    };

    g_update_call = [&]() {
        ridges_obj.toMesh(*w_mesh, false, false);
        ui.glwidget->update();
        flushAllEvents();
    };


    Graph graph;
    createGraph(maze_obj, ridges_obj.m_lasthelst, graph);
    mazeDfs(maze_obj);

    ridges_obj.toMesh(*w_mesh, false, false);
    ui.glwidget->update();

}

void MazeBalls::run() {
    //std::thread(&MazeBalls::run_thread, this).detach();
    run_thread();
}