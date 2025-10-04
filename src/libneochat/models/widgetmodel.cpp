// SPDX-FileCopyrightText: 2025 Arno Rehn <arno@arnorehn.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "widgetmodel.h"

#include <Quotient/jobs/downloadfilejob.h>
#include <Quotient/user.h>
#include <widgetevent.h>

using namespace Quotient;

namespace
{

// This URL will take us to a jitsi room that is configured on a best effort basis.
// Critically, it appears that we cannot pass avatars via the "fragment API".
// The important stuff is working though: conference id, user name, subject.
//
// The alternative would be to rely on a Jitsi wrapper like https://app.element.io/jitsi.html.
// This one parses the fragment and then uses the the actual Jitsi IFrame API to set
// up the Jitsi meeting. Nice, but ties us to that external wrapper.
//
// Or we ship our own wrapper as a local HTML file and open that one?
constexpr char JitsiMeetUrlTemplate[] =
    "https://$domain/$conferenceId"
    "#jitsi_meet_external_api_id=0"
    "&config.subject=\"$roomName\""
    "&config.startAudioOnly=$isAudioOnly"
    "&config.startWithAudioMuted=$startWithAudioMuted"
    "&config.startWithVideoMuted=$startWithVideoMuted"
    "&isVideoChannel=$isVideoChannel"
    "&userInfo.displayName=\"$matrix_display_name\"";

// Same thing with etherpad, as long as it's on scalar.vector.im
constexpr char EtherpadScalarVectorImUrl[] = "https://scalar.vector.im/api/widgets/etherpad.html";
constexpr char EtherpadUrlTemplate[] =
    "https://etherpad.integrations.element.io/p/"
    "$padName"
    "?showControls=true"
    "&showChat=false"
    "&chatAndUsers=false"
    "&alwaysShowChat=false"
    "&showLineNumbers=true"
    "&useMonospaceFont=false"
    "&userName=$matrix_user_id";

QUrl avatarUrl(NeoChatRoom *room)
{
    const auto avatarUrl = room->member(room->connection()->userId()).avatarUrl();
    return room->connection()->getUrlForApi<DownloadFileJob>(avatarUrl);
}

// Many widgets require proper interfacing with integration managers, but this needs authentication
// with an integration manager: https://github.com/matrix-org/matrix-spec-proposals/blob/main/proposals/1961-integrations-auth.md
// Also requires accepting the terms: https://github.com/matrix-org/matrix-spec-proposals/blob/main/proposals/2140-terms-of-service-2.md
// Example:
// curl -X POST "https://scalar.vector.im/_matrix/integrations/v1/register" \
//    -H "Content-Type: application/json" \
//    --data '<OpenIdCredentials>'
// Then append the resulting json key-value pairs as query items to the widget url.
QUrl buildWidgetUrl(QByteArray templateString, NeoChatRoom *room, const QJsonObject &data)
{
    constexpr auto enc = [](const auto &s) {
        return QUrl::toPercentEncoding(s);
    };

    templateString.replace("$matrix_user_id"_L1, enc(room->connection()->userId()))
        .replace("$matrix_room_id"_L1, enc(room->id()))
        .replace("$matrix_display_name"_L1, enc(room->member(room->connection()->userId()).displayName()))
        .replace("$matrix_avatar_url"_L1, avatarUrl(room).toEncoded());

    for (auto it = data.begin(); it != data.end(); ++it) {
        const QByteArray key = '$' + it.key().toUtf8();
        templateString.replace(key, enc(it.value().toString()));
    }

    return QUrl::fromEncoded(templateString);
}

}

class WidgetModelPrivate
{
    Q_DISABLE_COPY(WidgetModelPrivate);

public:
    WidgetModelPrivate(WidgetModel *q)
        : q_ptr(q)
    {
    }

    Q_DECLARE_PUBLIC(WidgetModel)
    WidgetModel *const q_ptr;

    // clang-format off
    struct NoModelResetTag_t {};
    static constexpr NoModelResetTag_t NoModelReset {};
    // clang-format on

    void reload();
    void reload(NoModelResetTag_t);
    void updateWidgets(Quotient::RoomEventsRange events);
    void handleEvent(const Quotient::RoomEvent *event);
    void handlePendingEvent(const Quotient::RoomEvent *event);
    void buildJitsiIndex();

    NeoChatRoom *room = nullptr;
    QMap<QString, const WidgetEvent *> state;
    int jitsiIndex = -1;
};

WidgetModel::WidgetModel(QObject *parent)
    : QAbstractListModel{parent}
    , d_ptr(std::make_unique<WidgetModelPrivate>(this))
{
}

WidgetModel::~WidgetModel()
{
}

QHash<int, QByteArray> WidgetModel::roleNames() const
{
    return {
        {TextRole, "text"},
        {UrlRole, "url"},
        {TypeRole, "type"},
    };
}

QVariant WidgetModel::data(const QModelIndex &index, int role) const
{
    Q_D(const WidgetModel);

    const int row = index.row();
    if (row < 0 || row >= d->state.count()) {
        return {};
    }

    const auto *ev = std::next(d->state.begin(), row).value();
    const auto type = ev->contentPart<QString>("type"_L1);
    const auto url = ev->contentPart<QString>("url"_L1).toUtf8();

    switch (role) {
    case TextRole:
        return ev->contentPart<QString>("name"_L1);
    case TypeRole:
        return type;
    case UrlRole: {
        QByteArray urlTemplate = url;
        if (type == "jitsi"_L1) {
            // Jitsi is special-cased even in Element. The URL that we get from the
            // widget is just "https://scalar.vector.im/api/widgets/jitsi.html" which
            // is not sufficient.
            urlTemplate = JitsiMeetUrlTemplate;
        } else if (type == "m.etherpad"_L1 && url.startsWith(EtherpadScalarVectorImUrl)) {
            // Etherpad is not special-cased, but the scalar.vector.im wrapper requires authentication
            // with the integration manager. We can just resolve this to the actual etherpad instance
            // to skip integration manager auth.
            urlTemplate = EtherpadUrlTemplate;
        }
        return buildWidgetUrl(urlTemplate, room(), ev->contentPart<QJsonObject>("data"_L1));
    }
    }

    return {};
}

int WidgetModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const WidgetModel);
    Q_UNUSED(parent);
    return d->state.count();
}

void WidgetModelPrivate::reload(NoModelResetTag_t)
{
    state.clear();

    if (room) {
        const auto events = room->currentState().eventsOfType(Quotient::WidgetEvent::MetaType.matrixId);
        for (auto ev : events) {
            handleEvent(ev);
        }
    }
}

void WidgetModelPrivate::reload()
{
    Q_Q(WidgetModel);

    q->beginResetModel();

    reload(NoModelReset);
    buildJitsiIndex();

    q->endResetModel();
}

void WidgetModelPrivate::updateWidgets(Quotient::RoomEventsRange events)
{
    using namespace std::ranges;

    constexpr auto isWidgetEvent = [](auto &&ev) {
        return ev->template is<WidgetEvent>();
    };

    if (any_of(events, isWidgetEvent)) {
        reload();
    }
}

void WidgetModelPrivate::handleEvent(const Quotient::RoomEvent *event)
{
    const WidgetEvent *widgetEvent = eventCast<const WidgetEvent>(event);
    if (!widgetEvent) {
        return;
    }

    auto it = state.find(widgetEvent->stateKey());
    if (it != state.end() && widgetEvent->replacedState() == it.value()->id()) {
        // we alrady have this and it should be...
        if (widgetEvent->contentJson().isEmpty()) {
            // ... removed because replacement is empty
            state.erase(it);
        } else {
            // replaced
            it.value() = widgetEvent;
        }
    } else if (!widgetEvent->contentJson().isEmpty()) {
        // insert if not empty
        it = state.insert(widgetEvent->stateKey(), widgetEvent);
    }
}

void WidgetModelPrivate::handlePendingEvent(const RoomEvent *event)
{
    Q_Q(WidgetModel);

    if (!event->is<WidgetEvent>()) {
        return;
    }

    q->beginResetModel();

    reload(NoModelReset);
    handleEvent(event);
    buildJitsiIndex();

    q->endResetModel();
}

void WidgetModelPrivate::buildJitsiIndex()
{
    Q_Q(WidgetModel);

    const auto newIndex = [&] -> int {
        auto it = std::find_if(state.cbegin(), state.cend(), [](const WidgetEvent *e) {
            return e->contentPart<QString>("type"_L1) == "jitsi"_L1;
        });

        if (it == state.cend()) {
            return -1;
        }

        return std::distance(state.cbegin(), it);
    }();

    if (newIndex != jitsiIndex) {
        jitsiIndex = newIndex;
        Q_EMIT q->jitsiIndexChanged();
    }
}

NeoChatRoom *WidgetModel::room() const
{
    Q_D(const WidgetModel);
    return d->room;
}

void WidgetModel::setRoom(NeoChatRoom *newRoom)
{
    Q_D(WidgetModel);

    if (d->room == newRoom) {
        return;
    }

    if (d->room) {
        disconnect(d->room, &NeoChatRoom::baseStateLoaded, this, nullptr);
        disconnect(d->room, &NeoChatRoom::aboutToAddNewMessages, this, nullptr);
        disconnect(d->room, &NeoChatRoom::pendingEventAboutToAdd, this, nullptr);
    }

    d->room = newRoom;

    if (d->room) {
        connect(d->room, &NeoChatRoom::baseStateLoaded, this, [d] {
            d->reload();
        });
        connect(d->room, &NeoChatRoom::aboutToAddNewMessages, this, [d](Quotient::RoomEventsRange events) {
            d->updateWidgets(events);
        });
        connect(d->room, &NeoChatRoom::pendingEventAboutToAdd, this, [d](Quotient::RoomEvent *event) {
            d->handlePendingEvent(event);
        });
    }
    d->reload();

    Q_EMIT roomChanged();
}

int WidgetModel::jitsiIndex() const
{
    Q_D(const WidgetModel);

    return d->jitsiIndex;
}
