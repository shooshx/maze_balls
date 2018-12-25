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
    Params()
        : cross_section(false, this)
        , ridge_amplitude(0.11, this)
        , rand_seed(0, this)
        , straigt_steps(2, this)
        , base_shape(EBaseShape::icosahedron, this)
        , start_divs(1, this)
        , post_divs(3, this)
    {}
    AutoParam<bool> cross_section;
    AutoParam<float> ridge_amplitude;
    AutoParam<int> rand_seed;
    AutoParam<int> straigt_steps; // after how many steps randomize direction
    AutoParam<EBaseShape> base_shape;
    AutoParam<int> start_divs;  // how many subdivisions to do before DFS
    AutoParam<int> post_divs;   // subdivs to do for smoothing the result
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
    void call_update_mesh();

signals:
    void update_mesh();

private:
    Ui::MazeBallsClass ui;
    MeshControl* m_meshHandler;

public:
    Params m_params;
};
