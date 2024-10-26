#include "JsonConverter.h"

#include "JsonNetworkManager.h"

JsonNetworkManager::JsonNetworkManager(const QString &host,
                                       int port,
                                       bool isHttps,
                                       QSslConfiguration *sslConfiguration)
: mHttpManager(host, port, isHttps, sslConfiguration)
{
}

void JsonNetworkManager::sendRequest(HttpManager::Method method,
                                     const QString &apiString,
                                     const completeFunctionType &onCompleteCallback,
                                     const errorFunctionType &onErrorCallback,
                                     const QVariantMap &data,
                                     const QVariantMap &queries,
                                     const QVariantMap &headers)
{
    auto onComplete = [onCompleteCallback, onErrorCallback](
                          const QByteArray &data,
                          const QList<QNetworkReply::RawHeaderPair> &headerPairs) {
        // Parse reply body
        QJsonParseError parseError;
        QJsonObject json = JsonConverter::parse(data, parseError);

        // Debug only
        QJsonDocument doc(json);
        QString strJson = doc.toJson(QJsonDocument::Indented);
        qDebug() << "**** Response in JSON:";
        qDebug().noquote() << QString::fromUtf8(strJson.toStdString().c_str());

        // Check JSON parse error
        if (parseError.error != QJsonParseError::NoError) {
            QString errorString = parseError.errorString();
            qWarning() << "Json parse error. replyText:" << data
                       << "errorString:" << errorString;

            onErrorCallback(errorString);
            return;
        }

        onCompleteCallback(json, headerPairs);
    };

    QByteArray jsonData = JsonConverter::qVariantMapToJson(data);

    mHttpManager.sendRequest(method,
                             apiString,
                             onComplete,
                             onErrorCallback,
                             jsonData,
                             queries,
                             headers);
}
