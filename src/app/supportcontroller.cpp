// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "supportcontroller.h"

#include <Quotient/csapi/support.h>

#include <QDebug>

using namespace Quotient;

void SupportController::setConnection(NeoChatConnection *connection)
{
    if (m_connection != connection) {
        m_connection = connection;
        Q_EMIT connectionChanged();

        load();
    }
}

NeoChatConnection *SupportController::connection() const
{
    return m_connection;
}

QString SupportController::supportPage() const
{
    return m_supportPage;
}

QList<SupportContact> SupportController::contacts() const
{
    return m_contacts;
}

void SupportController::load()
{
    if (!m_connection) {
        qWarning() << "Tried to load support information without a valid connection?";
        return;
    }

    m_connection->callApi<GetWellknownSupportJob>()
        .onResult([this](const auto &job) {
            m_supportPage = job->supportPage();
            m_contacts.reserve(job->contacts().size());
            for (const auto &contact : job->contacts()) {
                m_contacts.push_back(SupportContact{
                    .role = contact.role,
                    .matrixId = contact.matrixId,
                    .emailAddress = contact.emailAddress,
                });
            }

            Q_EMIT loaded();
        })
        .onFailure([this](const auto &job) {
            Q_UNUSED(job)

            // Just do nothing, our properties will be empty.
            Q_EMIT loaded();
        });
}

#include "moc_supportcontroller.cpp"
