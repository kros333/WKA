#ifndef SESSION_H
#define SESSION_H

#include <QJsonObject>
#include "common.h"
#include "satellite.h"
#include <QDateTime>

/// Сеанс
struct Session
{
    Session(QJsonObject sessionJson);
    Session();

    QJsonObject toJson();

    quint64 id;
    Satellite* ka;
    QDateTime visibilityStart;
    QDateTime visibilityEnd;
    QDateTime factSessionTimeStart;
    QDateTime factSessionTimeEnd;
    CompletionState cState;
    int answers;
    int numberOfSession; //порядковый номер сессии внутри окна видимости (не может быть более 5)
};

#endif // SESSION_H
