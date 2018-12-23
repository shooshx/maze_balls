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

private slots:
    void on_runBut_clicked();

private:
    Ui::MazeBallsClass ui;
    MeshControl* m_meshHandler;
};
