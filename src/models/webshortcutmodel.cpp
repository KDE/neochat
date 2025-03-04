// SPDX-FileCopyrightText: 2010 Eike Hein <hein@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "webshortcutmodel.h"

#ifdef HAVE_KIO
#include <KIO/CommandLauncherJob>
#include <KUriFilter>
#endif
#include <KStringHandler>

using namespace Qt::StringLiterals;

struct WebShortcutModelPrivate {
    QString selectedText;
#ifdef HAVE_KIO
    KUriFilterData filterData;
#endif
    QStringList searchProviders;
};

WebShortcutModel::WebShortcutModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new WebShortcutModelPrivate)
{
}

WebShortcutModel::~WebShortcutModel()
{
}

QString WebShortcutModel::selectedText() const
{
    return d->selectedText;
}

QString WebShortcutModel::trunkatedSearchText() const
{
    return KStringHandler::rsqueeze(d->selectedText, 21);
}

bool WebShortcutModel::enabled() const
{
#ifdef HAVE_KIO
    return true;
#else
    return false;
#endif
}

void WebShortcutModel::setSelectedText(const QString &selectedText)
{
    if (d->selectedText == selectedText) {
        return;
    }
#ifdef HAVE_KIO
    beginResetModel();
    d->selectedText = selectedText;

    if (selectedText.isEmpty()) {
        endResetModel();
        return;
    }

    QString searchText = selectedText;
    searchText = searchText.replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('\r'), QLatin1Char(' ')).simplified();
    if (searchText.isEmpty()) {
        endResetModel();
        return;
    }
    d->filterData.setData(searchText);
    d->filterData.setSearchFilteringOptions(KUriFilterData::RetrievePreferredSearchProvidersOnly);

    if (KUriFilter::self()->filterSearchUri(d->filterData, KUriFilter::NormalTextFilter)) {
        d->searchProviders = d->filterData.preferredSearchProviders();
    }
    endResetModel();
#else
    d->selectedText = selectedText;
#endif
    Q_EMIT selectedTextChanged();
}

int WebShortcutModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
#ifdef HAVE_KIO
    if (!d->selectedText.isEmpty()) {
        return d->searchProviders.count();
    }
#endif
    return 0;
}

QVariant WebShortcutModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

#ifdef HAVE_KIO
    switch (role) {
    case Qt::DisplayRole:
        return d->searchProviders[index.row()];
    case Qt::DecorationRole:
        return d->filterData.iconNameForPreferredSearchProvider(d->searchProviders[index.row()]);
    case Qt::EditRole:
        return d->filterData.queryForPreferredSearchProvider(d->searchProviders[index.row()]);
    }
#endif
    return {};
}

void WebShortcutModel::trigger(const QString &data)
{
#ifdef HAVE_KIO
    KUriFilterData filterData(data);
    if (KUriFilter::self()->filterSearchUri(filterData, KUriFilter::WebShortcutFilter)) {
        Q_EMIT openUrl(filterData.uri());
    }
#else
    Q_UNUSED(data);
#endif
}

void WebShortcutModel::configureWebShortcuts()
{
#ifdef HAVE_KIO
    auto job = new KIO::CommandLauncherJob(u"kcmshell6"_s, QStringList() << u"webshortcuts"_s, this);
    job->exec();
#endif
}

#include "moc_webshortcutmodel.cpp"
