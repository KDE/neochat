// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <memory>

struct KWebShortcutModelPrivate;

/**
 * @class KWebShortcutModel
 *
 * This class defines the model for listing web shortcuts for a specified selectedText.
 *
 * This can be used as follows in your QML code:
 *
 * ```qml
 * QQC2.Menu {
 *      id: webshortcutmenu
 *
 *      title: i18n("Search for '%1'", webshortcutmodel.trunkatedSearchText)
 *      property bool isVisible: selectedText && selectedText.length > 0 && webshortcutmodel.enabled
 *      Component.onCompleted: webshortcutmenu.parent.visible = isVisible
 *      onIsVisibleChanged: webshortcutmenu.parent.visible = isVisible
 *      Instantiator {
 *          model: WebShortcutModel {
 *              id: webshortcutmodel
 *              selectedText: loadRoot.selectedText
 *              onOpenUrl: Qt.openUrlExternally(url)
 *          }
 *          delegate: QQC2.MenuItem {
 *              text: model.display
 *              icon.name: model.decoration
 *              onTriggered: webshortcutmodel.trigger(model.edit)
 *          }
 *          onObjectAdded: webshortcutmenu.insertItem(0, object)
 *      }
 *      QQC2.MenuSeparator {}
 *      QQC2.MenuItem {
 *          text: i18n("Configure Web Shortcuts...")
 *          icon.name: "configure"
 *          onTriggered: webshortcutmodel.configureWebShortcuts()
 *      }
 *  }
 *  ```
 */
class KWebShortcutModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The text to find web shortcuts for.
     */
    Q_PROPERTY(QString selectedText READ selectedText WRITE setSelectedText NOTIFY selectedTextChanged)

    /**
     * @brief The selectedText elided at a set width.
     */
    Q_PROPERTY(QString trunkatedSearchText READ trunkatedSearchText NOTIFY selectedTextChanged)

    /**
     * @brief Whether web shortcuts are available.
     */
    Q_PROPERTY(bool enabled READ enabled CONSTANT)
public:
    explicit KWebShortcutModel(QObject *parent = nullptr);
    ~KWebShortcutModel();

    QString selectedText() const;
    void setSelectedText(const QString &selectedText);

    QString trunkatedSearchText() const;

    bool enabled() const;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief Trigger the openUrl signal for the given web shortcut.
     */
    Q_INVOKABLE void trigger(const QString &data);

    /**
     * @brief Request the menu for configuring web shortcut settings be opened.
     */
    Q_INVOKABLE void configureWebShortcuts();

Q_SIGNALS:
    void selectedTextChanged();
    void openUrl(const QUrl &url);

private:
    std::unique_ptr<KWebShortcutModelPrivate> d;
};
