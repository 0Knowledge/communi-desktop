/*
* Copyright (C) 2008-2014 The Communi Project
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include "overlay.h"
#include "bufferview.h"
#include "textbrowser.h"
#include <QStyleOptionToolButton>
#include <QPropertyAnimation>
#include <QStylePainter>
#include <IrcConnection>
#include <QToolButton>
#include <QShortcut>
#include <IrcBuffer>
#include <QPointer>
#include <QStyle>
#include <QEvent>

class OverlayButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(int rotation READ rotation WRITE setRotation)

public:
    OverlayButton(QWidget* parent) : QToolButton(parent)
    {
        d.rotation = 0;
    }

    bool isBusy() const
    {
        return d.animation && d.animation->state() == QAbstractAnimation::Running;
    }

    void setBusy(bool busy)
    {
        if (busy != isBusy()) {
            if (busy) {
                if (!d.animation) {
                    d.animation = new QPropertyAnimation(this, "rotation");
                    d.animation->setDuration(750);
                    d.animation->setStartValue(0);
                    d.animation->setEndValue(360);
                    d.animation->setLoopCount(-1);
                }
                d.animation->start(QAbstractAnimation::DeleteWhenStopped);
            } else if (d.animation) {
                QPropertyAnimation* finish = new QPropertyAnimation(this, "rotation");
                finish->setDuration(d.animation->duration() - d.animation->currentLoopTime());
                finish->setStartValue(d.animation->currentValue());
                finish->setEndValue(d.animation->endValue());
                finish->setLoopCount(1);
                finish->start(QAbstractAnimation::DeleteWhenStopped);
                d.animation->stop();
                delete d.animation;
            }
        }
    }

    int rotation() const
    {
        return d.rotation;
    }

    void setRotation(int rotation)
    {
        if (d.rotation != rotation) {
            d.rotation = rotation;
            update();
        }
    }

protected:
    void paintEvent(QPaintEvent*)
    {
        QStylePainter painter(this);
        QStyleOptionToolButton button;
        initStyleOption(&button);
        painter.translate(width() / 2, height() / 2);
        painter.rotate(d.rotation);
        painter.translate(-width() / 2, -height() / 2);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawComplexControl(QStyle::CC_ToolButton, button);
    }

private:
    struct Private {
        int rotation;
        QPointer<QPropertyAnimation> animation;
    } d;
};

Overlay::Overlay(BufferView* view) : QFrame(view)
{
    d.buffer = 0;
    d.view = view;

    d.button = new OverlayButton(view);
    connect(d.button, SIGNAL(clicked()), this, SLOT(reconnect()));

    d.shortcut = new QShortcut(Qt::Key_Space, view);
    d.shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(d.shortcut, SIGNAL(activated()), this, SLOT(reconnect()));

    setVisible(false);
    setFocusProxy(d.button);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    view->installEventFilter(this);

    connect(view, SIGNAL(bufferChanged(IrcBuffer*)), this, SLOT(init(IrcBuffer*)));
    init(view->buffer());
}

BufferView* Overlay::view() const
{
    return d.view;
}

bool Overlay::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);
    if (event->type() == QEvent::Resize)
        relayout();
    return false;
}

void Overlay::refresh()
{
    IrcConnection* connection = d.buffer ? d.buffer->connection() : 0;
    const bool connected = connection && connection->status() == IrcConnection::Connected;
    const bool connecting = connection && connection->status() == IrcConnection::Connecting;
    d.shortcut->setEnabled(!connected && !connecting);
    d.button->setVisible(!connected);
    d.button->setBusy(connecting);
    setVisible(!connected);
}

void Overlay::relayout()
{
    resize(parentWidget()->size());
    d.button->setGeometry(QStyle::alignedRect(layoutDirection(), Qt::AlignCenter, d.button->size(), rect()));
}

void Overlay::reconnect()
{
    IrcConnection* connection = d.buffer ? d.buffer->connection() : 0;
    if (connection) {
        connection->setEnabled(true);
        connection->open();
    }
}

void Overlay::init(IrcBuffer* buffer)
{
    if (d.buffer != buffer) {
        if (d.buffer) {
            IrcConnection* connection = d.buffer->connection();
            disconnect(connection, SIGNAL(statusChanged(IrcConnection::Status)), this, SLOT(refresh()));
        }
        d.buffer = buffer;
        if (buffer) {
            IrcConnection* connection = buffer->connection();
            connect(connection, SIGNAL(statusChanged(IrcConnection::Status)), this, SLOT(refresh()));
        }
        refresh();
    }
}

#include "overlay.moc"
