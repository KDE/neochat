#include <KColorSchemeManager>
#include <QAbstractItemModel>

#include "colorschemer.h"

ColorSchemer::ColorSchemer(QObject* parent)
    : QObject(parent)
    , c(new KColorSchemeManager(this))
{
}

ColorSchemer::~ColorSchemer()
{
}

QAbstractItemModel *ColorSchemer::model() const
{
    return c->model();
}

void ColorSchemer::apply(int idx)
{
    c->activateScheme(c->model()->index(idx, 0));
}

void ColorSchemer::apply(const QString &name)
{
    c->activateScheme(c->indexForScheme(name));
}

int ColorSchemer::indexForScheme(const QString &name) const
{
    return c->indexForScheme(name).row();
}

QString ColorSchemer::nameForIndex(int index) const
{
    return c->model()->data(c->model()->index(index, 0), Qt::DisplayRole).toString();
}
