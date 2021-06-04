#pragma once

#include <QObject>
#include <QIdentityProxyModel>

class QAbstractItemModel;
class KColorSchemeManager;

class ColorSchemer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)
public:
    ColorSchemer(QObject* parent = nullptr);
    ~ColorSchemer();

    QAbstractItemModel* model() const;
    Q_INVOKABLE void apply(int idx);
    Q_INVOKABLE void apply(const QString &name);
    Q_INVOKABLE int indexForScheme(const QString &name) const;
    Q_INVOKABLE QString nameForIndex(int index) const;

private:
    KColorSchemeManager* c;
};

