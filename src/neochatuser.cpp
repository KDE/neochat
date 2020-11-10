/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "neochatuser.h"

#include "csapi/profile.h"

QColor NeoChatUser::color()
{
    return QColor::fromHslF(hueF(), 0.7, 0.5, 1);
}
//TODO libQuotient 0.7: remove default name
void NeoChatUser::setDefaultName(QString defaultName)
{
    rename(defaultName);
    connect(this, &Quotient::User::defaultNameChanged, this, [this]() {
        m_defaultName = "";
        qDebug() << "asdf";
        Q_EMIT nameChanged();
    });
}

QString NeoChatUser::defaultName()
{
    if(m_defaultName.isEmpty()) {
        GetDisplayNameJob *job = connection()->callApi<GetDisplayNameJob>(id());
        connect(job, &BaseJob::success, this, [this, job] {
            if(job->displayname().isEmpty())
                m_defaultName = id();
            else
                m_defaultName = job->displayname();
            Q_EMIT nameChanged();
        });
    }
    return m_defaultName;
}
