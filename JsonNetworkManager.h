#pragma once

#include "HttpManager.h"

class JsonNetworkManager
{
public:
    using completeFunctionType = std::function<void(const QJsonObject &, const QList<QNetworkReply::RawHeaderPair> &)>;
    using errorFunctionType = std::function<void(const QString &)>;

public:
    explicit JsonNetworkManager(const QString &host,
                                int port,
                                bool isHttps,
                                QSslConfiguration *sslConfiguration = nullptr);

    void sendRequest(HttpManager::Method method,
                     const QString &apiString,
                     const completeFunctionType &onCompleteCallback,
                     const errorFunctionType &onErrorCallback,
                     const QVariantMap &data = QVariantMap(),
                     const QVariantMap &queries = QVariantMap(),
                     const QVariantMap &headers = QVariantMap());

private:
    HttpManager mHttpManager;
 };
