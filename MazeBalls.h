#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MazeBalls.h"
#include "MeshControl.h"
#include <memory>

class MeshControl;

class MazeBalls : public QMainWindow
{
    Q_OBJECT

public:
    MazeBalls(QWidget *parent = Q_NULLPTR);

    void run();
    void run_thread();

    void emit_update_mesh() {
        emit update_mesh();
    }

private slots:
    void on_runBut_clicked();
    void call_update_mesh();

signals:
    void update_mesh();

private:
    Ui::MazeBallsClass ui;
    MeshControl* m_meshHandler;
};
