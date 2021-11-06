// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "screencast.h"

class XScreenCast : public AbstractScreenCast
{
public:
    GstElement *request(int index) override;
    XScreenCast(QObject *parent);
    bool canShareScreen() const override;
    bool canSelectWindow() const override;
};
