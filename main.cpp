#include "MazeBalls.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MazeBalls w;
    w.show();
    return a.exec();
}
