#ifndef SATELLITE_H
#define SATELLITE_H

#include <QJsonObject>
#include <QDateTime>

struct Satellite
{
public:
    Satellite(QJsonObject satelliteJson);
    Satellite(int id, QString name);
    Satellite();

    QJsonObject toJson();

    int id;
    QString name;
    int countOfSpansPerDay;
    int posX;
    int posY;
};

#endif // SATELLITE_H
