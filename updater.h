#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QEventLoop>

class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject *parent = nullptr);
    void run();
    void stop();
    ~Updater();
    QPlainTextEdit *e;
    QEventLoop loop;

signals:

public slots:
    void update(QString message);

};

#endif // UPDATER_H
