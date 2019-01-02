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

    m_params.cross_section.mate(ui.crossCheck);
    m_params.ridge_amplitude.mate(ui.amplitudeSpinBox);
    m_params.rand_seed.mate(ui.randSeedSpinBox);
    m_params.straigt_steps.mate(ui.straightSpinBox);
    m_params.base_shape.mate(ui.shapeComboBox);
    m_params.start_divs.mate(ui.startDivsSpinBox);
    m_params.post_divs.mate(ui.postDivsSpinBox);
    m_params.save_quads.mate(ui.saveQuadsCheckBox);
    m_params.wireframe.mate(ui.wireCheckBox);
    m_params.color_maze.mate(ui.colorMazeCheckBox);
    m_params.tri_mesh.mate(ui.meshTriCheckBox);
    m_params.show_maze_mesh.mate(ui.mazeMeshCheckBox);
    m_params.smooth_start_div.mate(ui.smoothStartDivCheck);
    m_params.smooth_post_div.mate(ui.smoothPostDivCheck);

    connect(&m_params, &ParamAggregate::changed, [this]() {
        ui.glwidget->update();
    });

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
    // m_meshHandler->save("C:\\Projects\\maze_balls\\output.obj");

    m_meshHandler->m_meshes[0].m->save("C:\\Projects\\maze_balls\\sphere_0.obj", m_params.save_quads, false);
    m_meshHandler->m_meshes[1].m->save("C:\\Projects\\maze_balls\\sphere_1.obj", m_params.save_quads, true);

    m_meshHandler->m_meshes[0].m->saveJson("C:\\Projects\\maze_balls\\sphere_0.json", "sphere_0", false);
    m_meshHandler->m_meshes[1].m->saveJson("C:\\Projects\\maze_balls\\sphere_1.json", "sphere_1", false);
}

void MazeBalls::on_resetBut_clicked() {
    ui.glwidget->reset();
    m_params.resetToDefault();
    ui.glwidget->update();
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
    int direction_skips = 2;

    Params* params;
};

void recDfs(MyPolygon* p, DfsState* state) 
{
    p->visited = true;
    if (state->params->color_maze)
        p->f->col = Vec3(0.0, 0.0, 1.0);
    p->f->touched = true;

    //g_update_emit();
    HalfEdge* start_he = p->he;
    // after how many steps in this direction, randomize the direction
    if (state->params->straigt_steps > 0 && (state->count % state->params->straigt_steps) == 0) {
        state->direction_skips = rand() % 4;
    }
    ++state->count;
    for (int i = 0; i < state->direction_skips; ++i) {
        start_he = start_he->next;
    }

    HalfEdge* he = start_he;
    for(int i = 0; i < p->pnum; ++i) {
        he = he->next;
        if (!he->pair->poly->visited) {
            he->edge->walled = false;
            if (state->params->color_maze)
                he->edgePoint->col = Vec3(0.0, 0.0, 1.0);
            he->edgePoint->touched = true;
            recDfs(he->pair->poly, state);
        }
    }
    M_ASSERT(he == start_he); // sanity - reached around
}

void mazeDfs(MyObject& obj, Params* params) 
{
    DfsState state;
    state.params = params;
    recDfs(obj.poly[0], &state);
}

//#define OFFSET (0.085)
//#define OFFSET (0.11)

void raiseRidges(MyObject& obj, bool reverse, float amplitude, bool smooth)
{
    for(auto& pnt: obj.points) {
        Vec3 n = pnt->p.unitized();
        pnt->n = n;
        if (smooth) {
            if (pnt->touched == reverse) { // xor
                pnt->p = n * (1.0 + amplitude); // 0.9,0.75
            }
            else {
                pnt->p = n * (1.0 - amplitude);
            }
        }
        else {
            if (pnt->touched == reverse) { 
                pnt->p += n * amplitude;
            }
            else {
                pnt->p += n * -amplitude;
            }
        }
    }
}


void MazeBalls::make_ball(bool ridge_reverse, const Vec3& translate)
{
    unique_ptr<Mesh> load_mesh(new Mesh);
    const char* filename;
    if (m_params.base_shape == EBaseShape::icosahedron)
        filename = "C:\\Projects\\maze_balls\\src\\icosahedron.obj";
    else
        filename = "C:\\Projects\\maze_balls\\src\\cube.obj";
    load_mesh->loadObj(filename);
    load_mesh->m_uniformColor = true;

    MyAllocator obj_alloc(61443, 81921, 61441);
    MyObject maze_obj(&obj_alloc); // the obj on which we DFS the maze
    //maze_obj.toMesh(*w_mesh, false, false);

    meshToObj(*load_mesh, maze_obj);
    //maze_obj.subdivide(true, true);
    for(int i = 0; i < m_params.start_divs; ++i)
        maze_obj.subdivide(m_params.smooth_start_div, true);

    MyObject ridges_obj = maze_obj; // the obj that has the vallies and ridges
    ridges_obj.subdivide(m_params.smooth_start_div, false);

    srand(m_params.rand_seed);
    Graph graph;
    createGraph(maze_obj, ridges_obj.m_lasthelst, graph);
    mazeDfs(maze_obj, &m_params);

    MyObject ridges_right = ridges_obj;
    raiseRidges(ridges_right, ridge_reverse, m_params.ridge_amplitude, m_params.smooth_start_div);

    for(int i = 0; i < m_params.post_divs; ++i)
        ridges_right.subdivide(m_params.smooth_post_div, true);

    Mesh* w_mesh = new Mesh;
    w_mesh->m_uniformColor = true;
    MyObject* obj_to_mesh = m_params.show_maze_mesh ? &maze_obj : &ridges_right;
    obj_to_mesh->toMesh(*w_mesh, m_params.tri_mesh, true, ridge_reverse);
    

    m_meshHandler->addMesh(w_mesh, translate);

    obj_alloc.clear();
    obj_alloc.checkMaxAlloc();


}

void MazeBalls::run() {
    //std::thread(&MazeBalls::run_thread, this).detach();
    //run_thread();
    cout << "running!" << endl;
    m_meshHandler->clearMeshes();

    make_ball(false, Vec3(1,0,0));
    make_ball(true, Vec3(1,0,0));

    ui.glwidget->update();

}