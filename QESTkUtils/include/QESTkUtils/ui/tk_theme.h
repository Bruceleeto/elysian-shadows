/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef TK_THEME_H
#define TK_THEME_H

#include <QColor>

/* The editor's look: the Fusion style with a light or dark palette.
 * Widgets should take their colours from the palette. The status colours below
 * are mid-tones that stay readable on both themes, so items coloured with them
 * don't need recolouring when the theme changes. */
namespace Theme {

    enum class Mode { Light, Dark };

    void    apply(Mode mode);
    Mode    mode(void);
    bool    isDark(void);

    //Plain text in the current theme (for resetting a coloured item).
    QColor  text(void);
    QColor  dimText(void);
    QColor  errorText(void);
    QColor  warningText(void);
    QColor  scriptText(void);
    QColor  commandText(void);
}

#endif // TK_THEME_H
