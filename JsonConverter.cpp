#include <QJsonDocument>
#include <QJsonArray>

#include "JsonConverter.h"

namespace JsonConverter
{

QByteArray qVariantMapToJson(QVariantMap map)
{
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(map);
    QByteArray jsonData = jsonDocument.toJson();

    return jsonData;
}

QVariantMap jsonToVariantMap(const QByteArray &jsonData)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    QVariantMap map = jsonDocument.toVariant().toMap();

    return map;
}

QJsonObject parse(const QByteArray &data, QJsonParseError &parseError)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError)
        return QJsonObject();

    QJsonObject json;
    if (jsonDocument.isObject())
        json = jsonDocument.object();
    else if (jsonDocument.isArray())
        json[MagicArrayKey] = jsonDocument.array();

    return json;
}

} // namespace JsonConverter
