// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <memory>

struct KWebShortcutModelPrivate;

//! List model of web shortcuts for a a specified selectedText.
//!
//! This can be used as following in your QML code
//!
//! ```qml
//! QQC2.Menu {
//!     id: webshortcutmenu
//!     title: i18n("Search for '%1'", webshortcutmodel.trunkatedSearchText)
//!     property bool isVisible: selectedText && selectedText.length > 0 && webshortcutmodel.enabled
//!     Component.onCompleted: webshortcutmenu.parent.visible = isVisible
//!     onIsVisibleChanged: webshortcutmenu.parent.visible = isVisible
//!     Instantiator {
//!         model: WebShortcutModel {
//!             id: webshortcutmodel
//!             selectedText: loadRoot.selectedText
//!             onOpenUrl: Qt.openUrlExternally(url)
//!         }
//!         delegate: QQC2.MenuItem {
//!             text: model.display
//!             icon.name: model.decoration
//!             onTriggered: webshortcutmodel.trigger(model.edit)
//!         }
//!         onObjectAdded: webshortcutmenu.insertItem(0, object)
//!     }
//!     QQC2.MenuSeparator {}
//!     QQC2.MenuItem {
//!         text: i18n("Configure Web Shortcuts...")
//!         icon.name: "configure"
//!         onTriggered: webshortcutmodel.configureWebShortcuts()
//!     }
//! }
//! ```
class KWebShortcutModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString selectedText READ selectedText WRITE setSelectedText NOTIFY selectedTextChanged)
    Q_PROPERTY(QString trunkatedSearchText READ trunkatedSearchText NOTIFY selectedTextChanged)
    Q_PROPERTY(bool enabled READ enabled CONSTANT)
public:
    explicit KWebShortcutModel(QObject *parent = nullptr);
    ~KWebShortcutModel();

    QString selectedText() const;
    void setSelectedText(const QString &selectedText);
    QString trunkatedSearchText() const;
    bool enabled() const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Q_INVOKABLE void trigger(const QString &data);
    Q_INVOKABLE void configureWebShortcuts();
Q_SIGNALS:
    void selectedTextChanged();
    void openUrl(const QUrl &url);
private:
    std::unique_ptr<KWebShortcutModelPrivate> d;
};
