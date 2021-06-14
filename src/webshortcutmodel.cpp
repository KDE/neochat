// SPDX-FileCopyrightText: 2010 Eike Hein <hein@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "webshortcutmodel.h"

#ifdef HAVE_KIO
#include <KUriFilter>
#include <KIO/CommandLauncherJob>
#endif
#include <KStringHandler>

struct KWebShortcutModelPrivate
{
    QString selectedText;
#ifdef HAVE_KIO
    KUriFilterData filterData;
#endif
    QStringList searchProviders;
};

KWebShortcutModel::KWebShortcutModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new KWebShortcutModelPrivate)
{
}

KWebShortcutModel::~KWebShortcutModel()
{
}


QString KWebShortcutModel::selectedText() const
{
    return d->selectedText;
}

QString KWebShortcutModel::trunkatedSearchText() const
{
    return KStringHandler::rsqueeze(d->selectedText, 21);
}

bool KWebShortcutModel::enabled() const
{
#ifdef HAVE_KIO
    return true;
#else
    return false;
#endif
}

void KWebShortcutModel::setSelectedText(const QString &selectedText)
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
    searchText = searchText.replace('\n', ' ').replace('\r', ' ').simplified();
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

int KWebShortcutModel::rowCount(const QModelIndex &parent) const
{
#ifdef HAVE_KIO
    if (d->selectedText.count() > 0) {
        return d->searchProviders.count();
    }
#endif
    return 0;
}

QVariant KWebShortcutModel::data(const QModelIndex &index, int role) const
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

void KWebShortcutModel::trigger(const QString &data)
{
#ifdef HAVE_KIO
     KUriFilterData filterData(data);
     if (KUriFilter::self()->filterSearchUri(filterData, KUriFilter::WebShortcutFilter)) {
        Q_EMIT openUrl(filterData.uri().url());
     }
#else
    Q_UNUSED(data);
#endif
}

void KWebShortcutModel::configureWebShortcuts()
{
#ifdef HAVE_KIO
     auto job = new KIO::CommandLauncherJob("kcmshell5", QStringList() << "webshortcuts", this);
     job->exec();
#endif
}
