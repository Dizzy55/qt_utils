#pragma once

#include <QtGlobal>
#include <QMetaEnum>

namespace MetaEnum {

template <typename SomeEnum>
QString enumToString(SomeEnum key)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<SomeEnum>();
    return metaEnum.valueToKey(key);
}

template <typename SomeEnum>
SomeEnum enumFromString(const QString &key)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<SomeEnum>();
    const int index = metaEnum.keyToValue(key.toStdString().c_str());
    if (index == -1)
        Q_ASSERT_X(false, __func__, "Invalid enum string value");

    return index;
}

} // namespace MetaEnum
