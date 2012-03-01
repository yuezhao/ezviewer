#ifndef FLOATMANAGER_H
#define FLOATMANAGER_H

#include <QObject>

class FloatManager : public QObject
{
    Q_OBJECT
public:
    explicit FloatManager(QObject *parent = 0);

    void addWidget(QWidget *w);
    void cancelWidget(QWidget *w);

signals:

public slots:

};

#endif // FLOATMANAGER_H
