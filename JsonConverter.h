#pragma once

#include <QString>
#include <QByteArray>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>

namespace JsonConverter
{

const QString MagicArrayKey("magic_array_key");

QByteArray qVariantMapToJson(QVariantMap map);

QVariantMap jsonToVariantMap(const QByteArray &jsonData);

QJsonObject parse(const QByteArray &data, QJsonParseError &parseError);

template<class T>
QList<T> parseToList(const QJsonArray &jsonArray)
{
    QList<T> list;

    for (const QJsonValue &jsonValue : jsonArray)
        list.push_back(T::fromJson(jsonValue.toObject()));

    return list;
}

} // namespace JsonConverter
