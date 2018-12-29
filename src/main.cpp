#include "MazeBalls.h"
#include <QtWidgets/QApplication>

namespace topo {
    int main_topo_x();
}

int main(int argc, char *argv[])
{
    //topo::main_topo_x();

    QApplication a(argc, argv);
    MazeBalls w;
    w.show();
    return a.exec();
}
