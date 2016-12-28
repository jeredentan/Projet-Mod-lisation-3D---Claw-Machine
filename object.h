#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>

class object : public QObject
{
    Q_OBJECT
public:
    explicit object(QObject *parent = 0);

signals:

public slots:
};

#endif // OBJECT_H