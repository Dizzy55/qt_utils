#pragma once

#include <functional>

#include "HttpManager.h"

class JsonNetworkManager
{
public:
    using completeFunctionType = std::function<void(const QVariantMap &,
                                                    const QList<QPair<QByteArray, QByteArray>> &)>;
    using errorFunctionType = std::function<void(QNetworkReply::NetworkError,
                                                 int,
                                                 const QString &)>;

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
