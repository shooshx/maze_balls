#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MazeBalls.h"
#include "MeshControl.h"
#include <memory>

class MeshControl;

enum class EBaseShape {
    icosahedron,
    cube
};
ENUM_STRINGS(EBaseShape, "icosahedron", "cube")


class Params : public ParamAggregate {
public:
    Params() : ParamAggregate("ShyApp", "maze_balls") { loadFromReg();}

    AUTO_PARAM(bool, cross_section, false);
    AUTO_PARAM(float, ridge_amplitude, 0.11);
    AUTO_PARAM(int, rand_seed, 0);
    AUTO_PARAM(int, straigt_steps, 2); // after how many steps randomize direction
    AUTO_PARAM(EBaseShape, base_shape, EBaseShape::icosahedron);
    AUTO_PARAM(int, start_divs, 2);  // how many subdivisions to do before DFS
    AUTO_PARAM(int, post_divs, 3);   // subdivs to do for smoothing the result
    AUTO_PARAM(bool, save_quads, true);
    AUTO_PARAM(bool, wireframe, false);
    AUTO_PARAM(bool, color_maze, false);
    AUTO_PARAM(bool, tri_mesh, false);
    AUTO_PARAM(bool, show_maze_mesh, false);
    AUTO_PARAM(bool, smooth_start_div, true);
    AUTO_PARAM(bool, smooth_post_div, true);
};

class MazeBalls : public QMainWindow
{
    Q_OBJECT
public:
    MazeBalls(QWidget *parent = Q_NULLPTR);

    void run();
    void make_ball(bool ridge_reverse, const Vec3& translate);

    void emit_update_mesh() {
        emit update_mesh();
    }

private slots:
    void on_runBut_clicked();
    void on_saveBut_clicked();
    void on_resetBut_clicked();
    void call_update_mesh();

signals:
    void update_mesh();

private:
    Ui::MazeBallsClass ui;
    MeshControl* m_meshHandler;

public:
    Params m_params;
};
