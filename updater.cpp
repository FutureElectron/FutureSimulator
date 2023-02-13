#include "updater.h"

Updater::Updater(QObject *parent)
    : QObject{parent}
{
    qDebug()<<"thread created";
}

void Updater::run()
{
    qDebug()<<"Thread started";
    loop.exec();
}

void Updater::stop()
{
    qDebug()<<"Thread stopped";
    if(loop.isRunning()) loop.quit();
}

Updater::~Updater()
{
    if(loop.isRunning()) loop.quit();
}

void Updater::update(QString message)
{
//    e->appendPlainText(message);
    qDebug("Appended message");
}
