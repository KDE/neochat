// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QQmlEngine>

#include <Quotient/converters.h>
#include <Quotient/events/eventrelation.h>
#include <Quotient/events/roomevent.h>
#include <Quotient/quotient_common.h>

using namespace Qt::StringLiterals;

/**
 * @class PollKind
 *
 * This class is designed to define the PollKind enumeration.
 */
class PollKind : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Enum representing the available poll kinds.
     */
    enum Kind {
        Disclosed, /**< The poll results can been seen after the user votes. */
        Undisclosed, /**< The poll results can only been seen after the poll ends. */
    };
    Q_ENUM(Kind);

    /**
     * @brief Return the string for the given Kind.
     *
     * @sa Kind
     */
    static QString stringForKind(Kind kind)
    {
        switch (kind) {
        case Undisclosed:
            return "org.matrix.msc3381.poll.undisclosed"_L1;
        default:
            return "org.matrix.msc3381.poll.disclosed"_L1;
        }
    }

    /**
     * @brief Return the Kind for the given string.
     *
     * @sa Kind
     */
    static Kind kindForString(const QString &kindString)
    {
        if (kindString == "org.matrix.msc3381.poll.undisclosed"_L1) {
            return Undisclosed;
        }
        return Disclosed;
    }
};

namespace Quotient
{
namespace EventContent
{

/**
 * @brief An answer to the poll.
 */
struct Answer {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id CONSTANT)
    Q_PROPERTY(QString text MEMBER text CONSTANT)

public:
    QString id;
    QString text;

    int operator==(const Answer &right) const
    {
        return id == right.id && text == right.text;
    }
};

/**
 * @brief Struct representing the content of a poll event.
 */
struct PollStartContent {
    PollKind::Kind kind;
    int maxSelection;
    QString question;
    QList<EventContent::Answer> answers;
};

} // namespace EventContent

template<>
inline EventContent::Answer fromJson(const QJsonObject &jo)
{
    return EventContent::Answer{fromJson<QString>(jo["id"_L1]), fromJson<QString>(jo["org.matrix.msc1767.text"_L1])};
}

template<>
inline auto toJson(const EventContent::Answer &c)
{
    QJsonObject jo;
    addParam<IfNotEmpty>(jo, "id"_L1, c.id);
    addParam<IfNotEmpty>(jo, "org.matrix.msc1767.text"_L1, c.text);
    return jo;
}

template<>
inline EventContent::PollStartContent fromJson(const QJsonObject &jo)
{
    return EventContent::PollStartContent{
        PollKind::kindForString(jo["org.matrix.msc3381.poll.start"_L1]["kind"_L1].toString()),
        fromJson<int>(jo["org.matrix.msc3381.poll.start"_L1]["max_selections"_L1]),
        fromJson<QString>(jo["org.matrix.msc3381.poll.start"_L1]["question"_L1]["org.matrix.msc1767.text"_L1]),
        fromJson<QList<EventContent::Answer>>(jo["org.matrix.msc3381.poll.start"_L1]["answers"_L1]),
    };
}

template<>
inline auto toJson(const EventContent::PollStartContent &c)
{
    QJsonObject innerJo;
    addParam<IfNotEmpty>(innerJo, "kind"_L1, PollKind::stringForKind(c.kind));
    addParam(innerJo, "max_selections"_L1, c.maxSelection);
    if (innerJo["max_selections"_L1].toInt() < 1) {
        innerJo["max_selections"_L1] = 1;
    }
    innerJo.insert("question"_L1, QJsonObject{{"org.matrix.msc1767.text"_L1, c.question}});
    addParam<IfNotEmpty>(innerJo, "answers"_L1, c.answers);

    QJsonObject jo;
    auto textString = c.question;
    for (int i = 0; i < c.answers.length(); ++i) {
        textString.append("\n%1. %2"_L1.arg(QString::number(i + 1), c.answers.at(i).text));
    }
    addParam<IfNotEmpty>(jo, "org.matrix.msc1767.text"_L1, textString);
    jo.insert("org.matrix.msc3381.poll.start"_L1, innerJo);
    return jo;
}

/**
 * @class PollStartEvent
 *
 * Class to define a poll start event.
 *
 * See MSC3381 for full details on polls in the matrix spec
 * https://github.com/matrix-org/matrix-spec-proposals/blob/travis/msc/polls/proposals/3381-polls.md.
 *
 * @sa Quotient::RoomEvent
 */
class PollStartEvent : public EventTemplate<PollStartEvent, RoomEvent, EventContent::PollStartContent>
{
public:
    QUO_EVENT(PollStartEvent, "org.matrix.msc3381.poll.start");
    using EventTemplate::EventTemplate;

    /**
     * @brief The poll kind.
     */
    PollKind::Kind kind() const;

    /**
     * @brief The maximum number of options a user can select in a poll.
     */
    int maxSelections() const;

    /**
     * @brief The question being asked in the poll.
     */
    QString question() const;

    /**
     * @brief The list of answers to the poll.
     */
    QList<EventContent::Answer> answers() const;
};

/**
 * @class PollResponseEvent
 *
 * Class to define a poll response event.
 *
 * See MSC3381 for full details on polls in the matrix spec
 * https://github.com/matrix-org/matrix-spec-proposals/blob/travis/msc/polls/proposals/3381-polls.md.
 *
 * @sa Quotient::RoomEvent
 */
class PollResponseEvent : public RoomEvent
{
public:
    QUO_EVENT(PollResponseEvent, "org.matrix.msc3381.poll.response");
    explicit PollResponseEvent(const QJsonObject &obj);
    explicit PollResponseEvent(const QString &pollStartEventId, QStringList responses);

    /**
     * @brief The selected answers to the poll.
     */
    QStringList selections() const;

    /**
     * @brief The EventRelation pointing to the PollStartEvent.
     */
    std::optional<EventRelation> relatesTo() const;
};

/**
 * @class PollEndEvent
 *
 * Class to define a poll end event.
 *
 * See MSC3381 for full details on polls in the matrix spec
 * https://github.com/matrix-org/matrix-spec-proposals/blob/travis/msc/polls/proposals/3381-polls.md.
 *
 * @sa Quotient::RoomEvent
 */
class PollEndEvent : public RoomEvent
{
public:
    QUO_EVENT(PollEndEvent, "org.matrix.msc3381.poll.end");
    explicit PollEndEvent(const QJsonObject &obj);
    explicit PollEndEvent(const QString &pollStartEventId, const QString &endText);

    /**
     * @brief The EventRelation pointing to the PollStartEvent.
     */
    std::optional<EventRelation> relatesTo() const;
};
}
