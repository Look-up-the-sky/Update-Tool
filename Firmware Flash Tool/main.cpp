#include "updatewindow.h"
#include <QApplication>
#include <QObject>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UpdateWindow w;
    w.show();
    return a.exec();
}
