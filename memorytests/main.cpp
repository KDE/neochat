// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include "memtesttimelinemodel.h"
#include "models/messagefiltermodel.h"

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("neochat"));

    QQmlApplicationEngine engine;

    KLocalization::setupLocalizedContext(&engine);

    MemTestTimelineModel *memTestTimelineModel = new MemTestTimelineModel;
    MessageFilterModel *messageFilterModel = new MessageFilterModel(nullptr, memTestTimelineModel);
    engine.rootContext()->setContextProperty(u"memTestTimelineModel"_s, memTestTimelineModel);
    engine.rootContext()->setContextProperty(u"messageFilterModel"_s, messageFilterModel);

    engine.loadFromModule("org.kde.neochat.timeline_memtest", "Main");

    return app.exec();
}
