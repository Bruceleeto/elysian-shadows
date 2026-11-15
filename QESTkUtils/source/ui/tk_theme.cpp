/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <QESTkUtils/ui/tk_theme.h>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

namespace Theme {

namespace {
    Mode _mode = Mode::Light;

    //The editor's lavender selection, kept from the original look.
    const QColor lightHighlight(225, 202, 251);
    const QColor darkHighlight(112, 82, 166);

    QPalette _lightPalette(void) {
        QPalette p = QApplication::style()->standardPalette();
        p.setColor(QPalette::Highlight,       lightHighlight);
        p.setColor(QPalette::HighlightedText, Qt::black);
        return p;
    }

    QPalette _darkPalette(void) {
        const QColor window(45, 45, 48);
        const QColor base(30, 30, 32);
        const QColor alternateBase(40, 40, 43);
        const QColor button(55, 55, 59);
        const QColor text(220, 220, 220);
        const QColor disabledText(120, 120, 120);

        QPalette p;
        p.setColor(QPalette::Window,          window);
        p.setColor(QPalette::WindowText,      text);
        p.setColor(QPalette::Base,            base);
        p.setColor(QPalette::AlternateBase,   alternateBase);
        p.setColor(QPalette::ToolTipBase,     button);
        p.setColor(QPalette::ToolTipText,     text);
        p.setColor(QPalette::PlaceholderText, disabledText);
        p.setColor(QPalette::Text,            text);
        p.setColor(QPalette::Button,          button);
        p.setColor(QPalette::ButtonText,      text);
        p.setColor(QPalette::BrightText,      QColor(255, 90, 90));
        p.setColor(QPalette::Light,           QColor(75, 75, 80));
        p.setColor(QPalette::Midlight,        QColor(62, 62, 66));
        p.setColor(QPalette::Mid,             QColor(38, 38, 40));
        p.setColor(QPalette::Dark,            QColor(25, 25, 27));
        p.setColor(QPalette::Shadow,          QColor(10, 10, 10));
        p.setColor(QPalette::Link,            QColor(140, 170, 255));
        p.setColor(QPalette::LinkVisited,     QColor(180, 150, 240));
        p.setColor(QPalette::Highlight,       darkHighlight);
        p.setColor(QPalette::HighlightedText, Qt::white);

        p.setColor(QPalette::Disabled, QPalette::WindowText,      disabledText);
        p.setColor(QPalette::Disabled, QPalette::Text,            disabledText);
        p.setColor(QPalette::Disabled, QPalette::ButtonText,      disabledText);
        p.setColor(QPalette::Disabled, QPalette::Light,           window);
        p.setColor(QPalette::Disabled, QPalette::Highlight,       QColor(70, 70, 74));
        p.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledText);
        return p;
    }
}

void apply(Mode mode) {
    _mode = mode;
    if(QApplication::style()->name().compare("fusion", Qt::CaseInsensitive) != 0) {
        QApplication::setStyle(QStyleFactory::create("Fusion"));
    }
    QApplication::setPalette(mode == Mode::Dark? _darkPalette() : _lightPalette());
}

Mode mode(void) { return _mode; }

bool isDark(void) { return _mode == Mode::Dark; }

QColor text(void) { return QApplication::palette().color(QPalette::Text); }

QColor dimText(void) { return QColor(Qt::gray); }

QColor errorText(void) { return QColor(215, 60, 60); }

QColor warningText(void) { return QColor(200, 150, 0); }

QColor scriptText(void) { return QColor(0, 150, 170); }

QColor commandText(void) { return QColor(50, 160, 70); }

}
