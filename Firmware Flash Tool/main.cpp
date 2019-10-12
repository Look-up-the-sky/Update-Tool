#include "updatewindow.h"
#include <QApplication>
#include <QObject>
/************************
 * v1.0 创建 ——沈仕强
 * v1.1 修复LCD屏升级时的bug 2019.8.5
 *
 *
 * ***********************/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UpdateWindow w;
    w.show();
    return a.exec();
}
