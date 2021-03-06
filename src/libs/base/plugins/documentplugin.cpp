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

#include "documentplugin.h"
#include "bufferview.h"
#include <QApplication>
#include <QVariant>

// TODO:
static QWidget* findCommuniMainWindow()
{
    static QPointer<QWidget> window;
    if (!window) {
        foreach (QWidget* widget, QApplication::topLevelWidgets()) {
            if (widget->inherits("QMainWindow")) {
                window = widget;
                break;
            }
        }
    }
    return window;
}

TextDocument* DocumentPlugin::currentDocument() const
{
    QWidget* window = findCommuniMainWindow();
    if (window) {
        BufferView* view = window->property("currentView").value<BufferView*>();
        if (view)
            return view->textDocument();
    }
    return 0;
}
