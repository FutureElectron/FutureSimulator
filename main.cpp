#include "widget.h"
#include "logger.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Logger::attach();
    Logger::logging = true;
    Widget w;

    w.show();
    return a.exec();
}
