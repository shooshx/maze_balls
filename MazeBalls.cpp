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

void MazeBalls::on_saveBut_clicked()
{
    m_meshHandler->save("C:\\Projects\\maze_balls\\output.obj");
}

std::function<void(void)> g_update_emit;
std::function<void(void)> g_update_call;

void MazeBalls::call_update_mesh() {
    g_update_call();
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

struct DfsState {
    int count = 0;
};

void recDfs(MyPolygon* p, DfsState* state) 
{
    p->visited = true;
    //p->f->col = Vec3(1.0, 0.0, 0.0);
    p->f->touched = true;

    //g_update_emit();
    HalfEdge* start_he = p->he;
    if ((state->count % 2) == 0) {
        int skips = rand() % 4;
        for (int i = 0; i < skips; ++i) {
            start_he = start_he->next;
        }
    }
    ++state->count;

    HalfEdge* he = start_he;
    for(int i = 0; i < 4; ++i) {
        he = he->next;
        if (!he->pair->poly->visited) {
            he->edge->walled = false;
            //he->edgePoint->col = Vec3(1.0, 0.0, 0.0);
            he->edgePoint->touched = true;
            recDfs(he->pair->poly, state);
        }
    }
    M_ASSERT(he == start_he); // sanity - reached around
}

void mazeDfs(MyObject& obj) 
{
    DfsState state;
    recDfs(obj.poly[0], &state);
}

//#define OFFSET (0.085)
#define OFFSET (0.11)

void raiseRidges(MyObject& obj, bool reverse)
{
    for(auto& pnt: obj.points) {
        Vec3 n = pnt->p.unitized();
        pnt->n = n;
        if (pnt->touched == reverse) { // xor
            pnt->p = n * (1.0 + OFFSET); // 0.9,0.75
        }
        else {
            pnt->p = n * (1.0 - OFFSET);
        }
    }
}


void MazeBalls::make_ball(bool ridge_reverse, const Vec3& translate)
{
    unique_ptr<Mesh> load_mesh(new Mesh);
    const char* filename = "C:\\Projects\\maze_balls\\src\\icosahedron.obj";
    //const char* filename = "C:\\Projects\\maze_balls\\src\\cube.obj";
    load_mesh->loadObj(filename);
    load_mesh->m_uniformColor = true;

    MyAllocator obj_alloc;
    MyObject maze_obj(&obj_alloc); // the obj on which we DFS the maze
    //maze_obj.toMesh(*w_mesh, false, false);

    meshToObj(*load_mesh, maze_obj);
    maze_obj.subdivide(true, true);
    maze_obj.subdivide(true, true);
    //maze_obj.subdivide(true, true);
    //maze_obj.subdivide(true, true);

    MyObject ridges_obj = maze_obj; // the obj that has the vallies and ridges
    ridges_obj.subdivide(true, false);

    srand(0);
    Graph graph;
    createGraph(maze_obj, ridges_obj.m_lasthelst, graph);
    mazeDfs(maze_obj);

    MyObject ridges_right = ridges_obj;
    raiseRidges(ridges_right, ridge_reverse);

    ridges_right.subdivide(true, true);
    ridges_right.subdivide(true, true);
    ridges_right.subdivide(true, true);

    Mesh* w_mesh = new Mesh;
    w_mesh->m_uniformColor = true;
    ridges_right.toMesh(*w_mesh, false, true, ridge_reverse);
    w_mesh->calcQuadNormals(ridge_reverse);
    m_meshHandler->addMesh(w_mesh, translate);


    //w_mesh->save("C:\\Projects\\maze_balls\\output.obj", true);

}

void MazeBalls::run() {
    //std::thread(&MazeBalls::run_thread, this).detach();
    //run_thread();
    cout << "running!" << endl;
    m_meshHandler->clearMeshes();

    make_ball(false, Vec3(1,0,0));
    make_ball(true, Vec3(1,0,0));

    //m_meshHandler->m_meshes[0].m->save("C:\\Projects\\maze_balls\\output_0.obj", true);
    //m_meshHandler->m_meshes[1].m->save("C:\\Projects\\maze_balls\\output_1.obj", true);

    ui.glwidget->update();

}