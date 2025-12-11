#include "mainwindow.h"
#include <QApplication>

//start gameloop
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
