// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QString>

#include "block.h"
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
    Q_ENUM(Type)

    /**
     * @brief Set the string being handled.
     *
     * @note The TextHandler doesn't modify the input data variable so the unhandled
     *       text can always be retrieved.
     */
    void setData(const QString &string);

    /**
     * @brief Transform the given markdown string to HTML.
     */
    static void markdownToHtml(QString &string);

    /**
     * @brief If the given string has exactly 1 para remove the tags.
     */
    static void cleanParas(QString &string);

    /**
     * @brief Clean any html within the given string.
     *
     * This removes any invalid tags or attributes for matrix.
     */
    static void cleanHtml(QString &string);

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
                                  bool stripNewlines = false,
                                  bool isEdited = false,
                                  bool spoilerRevealed = false);

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

    /**
     * @brief Split the given string into Blocks.
     *
     * Separate blocks are used for thing like paragraphs, codeblocks and quotes.
     * Each block will have handleRecieveRichText() called on it.
     */
    Blocks::BlockPtrs textComponents(QString string,
                                     Qt::TextFormat inputFormat = Qt::RichText,
                                     const NeoChatRoom *room = nullptr,
                                     const Quotient::RoomEvent *event = nullptr,
                                     bool isEdited = false,
                                     bool spoilerRevealed = false,
                                     QObject *parent = nullptr);

    /**
     * @brief Modify the style parameters of the spoilers to reveal or hide the text.
     */
    static QString updateSpoilerText(QObject *object, QString string, bool spoilerRevealed);

    /**
     * @brief Strips Matrix links (matrix.to) from Markdown while not touching anything else.
     */
    static QString stripMatrixLinks(QString string);

    [[nodiscard]] static QString unescapeBackslashes(QString text);

private:
    QString m_data;

    QString m_dataBuffer;
    qsizetype m_pos;
    Type m_nextTokenType = Text;
    QString m_nextToken;

    static QString next(const QString &string, Type nextTokenType, qsizetype &pos);
    static Type getNextTokenType(const QString &string, qsizetype currentPos, const QString &currentToken, Type currentTokenType);

    qsizetype nextBlockPos(const QString &string) const;
    Blocks::Block *nextBlock(const QString &string,
                             qsizetype nextBlockPos,
                             Qt::TextFormat inputFormat = Qt::RichText,
                             const NeoChatRoom *room = nullptr,
                             const Quotient::RoomEvent *event = nullptr,
                             bool isEdited = false,
                             bool spoilerRevealed = false,
                             QObject *parent = nullptr);
    QString stripBlockTags(QString string, const QString &tagType) const;

    static QString getTagType(const QString &tagToken);
    static bool isCloseTag(const QString &tagToken);
    static QString getAttributeType(const QString &string);
    static QString getAttributeData(const QString &string, bool stripQuotes = false);
    static bool isAllowedTag(const QString &type);
    static bool isAllowedAttribute(const QString &tag, const QString &attribute);
    static bool isAllowedLink(const QString &link, bool isImg = false);
    static QString cleanAttributes(const QString &tag, const QString &tagString);
    QString addStyleToText(const QString &tag, QString cleanTagString, bool spoilerRevealed = false);
    QVariantMap getAttributes(const QString &tag, const QString &tagString);

    static QString cmarkdownToHtml(const QString &markdown);
    static QString escapeHtml(QString stringIn);
    QString unescapeHtml(QString stringIn);
    QString linkifyUrls(QString stringIn);
    static QString customMarkdownToHtml(const QString &stringIn);
    static QString fixupUnderlineSyntax(const QString &stringIn);
    static void processWithinHTML(QString &buffer, const QString &syntax, const QString &beginTag, const QString &endTag);
    static void processWithinMarkdown(QString &buffer, const QString &syntax, const QString &beginTag, const QString &endTag);
    static void escapeURLs(QString &stringIn);

    QString editString() const;
    QString emoteString(const NeoChatRoom *room = nullptr, const Quotient::RoomEvent *event = nullptr) const;

    static QString convertCodeLanguageString(const QString &languageString);
};
