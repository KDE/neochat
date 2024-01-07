// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QHash>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "neochatroom.h"

namespace Quotient
{
class RoomMessageEvent;
}

/**
 * @class TextHandler
 *
 * This class is designed to handle the text of both incoming and outgoing messages.
 *
 * This includes converting markdown to html and removing any html tags that shouldn't
 * be present as per the matrix spec
 * (https://spec.matrix.org/v1.5/client-server-api/#mroommessage-msgtypes).
 */
class TextHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief List of token types
     */
    enum Type {
        Text, /*!< Anything not a tag that doesn't have special handling */
        Tag, /*!< For any generic tag that doesn't have special handling */
        TextCode, /*!< Text between code tags */
        End, /*!< End of the input string */
    };

    /**
     * @brief Get the string being handled.
     *
     * Setting new data resets the TextHandler.
     */
    QString data() const;

    /**
     * @brief Set the string being handled.
     *
     * @note The TextHandler doesn't modify the input data variable so the unhandled
     *       text can always be retrieved.
     */
    void setData(const QString &string);

    /**
     * @brief Handle the text for a message that is being sent.
     */
    QString handleSendText();

    /**
     * @brief Handle the text as a rich output for a message being received.
     *
     * The function does the following:
     *  - Removes invalid html tags and attributes
     *  - Strips any reply from the message
     *  - Formats user mentions
     *
     * @note In this case the rich text refers to the output format. The input
     *       can be in either and the parameter inputFormat just needs to be set
     *       appropriately.
     */
    QString handleRecieveRichText(Qt::TextFormat inputFormat = Qt::RichText,
                                  const NeoChatRoom *room = nullptr,
                                  const Quotient::RoomEvent *event = nullptr,
                                  bool stripNewlines = false);

    /**
     * @brief Handle the text as a plain output for a message being received.
     *
     * The function does the following:
     *  - Removes all html tags and attributes (except inside of code tags)
     *  - Strips any reply from the message
     *
     * @note In this case the plain text refers to the output format. The input
     *       can be in either and the parameter inputFormat just needs to be set
     *       appropriately.
     *
     * @warning The output of this function should NEVER be input into a rich text
     *          control. It will try to preserve < and > in the plain string which
     *          could be malicious tags if the control uses rich text format.
     */
    QString handleRecievePlainText(Qt::TextFormat inputFormat = Qt::PlainText, const bool &stripNewlines = false);

private:
    QString m_data;

    QString m_dataBuffer;
    int m_pos;
    Type m_nextTokenType = Text;
    QString m_nextToken;

    void next();
    void nextTokenType();

    QString getTagType() const;
    bool isCloseTag() const;
    QString getAttributeType(const QString &string);
    QString getAttributeData(const QString &string);
    bool isAllowedTag(const QString &type);
    bool isAllowedAttribute(const QString &tag, const QString &attribute);
    bool isAllowedLink(const QString &link, bool isImg = false);
    QString cleanAttributes(const QString &tag, const QString &tagString);

    QString markdownToHTML(const QString &markdown);
    QString escapeHtml(QString stringIn);
    QString unescapeHtml(QString stringIn);
    QString linkifyUrls(QString stringIn);
};
