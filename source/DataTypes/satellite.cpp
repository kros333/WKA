#include "satellite.h"
#include <ctime>
#include "common.h"

Satellite::Satellite(int id, QString name)
{
    this->id = id;
    this->name = name;

    this->posX = getRandomInt(0, 19);
    this->posY = getRandomInt(0, 19);
    this->countOfSpansPerDay = getRandomInt(1, 4);
}

QJsonObject Satellite::toJson()
{
    QJsonObject satelliteJson;
    satelliteJson["id"] = id;
    satelliteJson["name"] = name;

    return satelliteJson;
}
