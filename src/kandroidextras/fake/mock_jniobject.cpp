/*
    SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mock_jniobject.h"

using namespace KAndroidExtras::Internal;

QStringList MockJniObjectBase::m_staticProtocol;

class MockJniObjectBasePrivate : public QSharedData
{
public:
    QStringList protocol;
    QHash<QByteArray, QVariant> properties;
    QVariant value;
};

MockJniObjectBase::MockJniObjectBase()
    : d(new MockJniObjectBasePrivate)
{
}

MockJniObjectBase::MockJniObjectBase(const char *className)
    : d(new MockJniObjectBasePrivate)
{
    addToProtocol(QLatin1StringView("ctor: ") + QLatin1StringView(className));
}

MockJniObjectBase::MockJniObjectBase(jobject object)
    : d(new MockJniObjectBasePrivate)
{
    Q_UNUSED(object);
    addToProtocol(QLatin1StringView("ctor: o"));
}

MockJniObjectBase::MockJniObjectBase(const MockJniObjectBase &) = default;
MockJniObjectBase &MockJniObjectBase::operator=(const MockJniObjectBase &) = default;
MockJniObjectBase::~MockJniObjectBase() = default;

QStringList MockJniObjectBase::protocol() const
{
    return d->protocol;
}

void MockJniObjectBase::addToProtocol(const QString &line) const
{
    d->protocol.push_back(line);
}

void MockJniObjectBase::setProtocol(const QStringList &protocol)
{
    d->protocol = protocol;
}

QVariant MockJniObjectBase::property(const QByteArray &name) const
{
    return d->properties.value(name);
}

void MockJniObjectBase::setProperty(const QByteArray &name, const QVariant &value)
{
    d->properties.insert(name, value);
}

QVariant MockJniObjectBase::value() const
{
    return d->value;
}

void MockJniObjectBase::setValue(const QVariant &value)
{
    d->value = value;
}

void MockJniObjectBase::setData(jobject object)
{
    d = reinterpret_cast<MockJniObjectBasePrivate *>(object);
}
