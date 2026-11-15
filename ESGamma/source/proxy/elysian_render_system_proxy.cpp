/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESGamma/proxy/elysian_render_system_proxy.hpp>

/*!
    \qmltype RenderSystem
    \since 1.6.1
    \inmodule Engine
    \brief Engine subsystem responsible for all graphics and rendering

    \section2 Post-Processing Effects Pipeline

    \section2 Retrieving the RenderSystem
    The RenderSystem object can be retrieved through the static \l{Engine} class:
    \code
    renderSys = Engine.getRenderSys()
    \endcode

    \sa Engine
*/


namespace elysian::lua {
using namespace elysian;
using RenderSystem = elysian::RenderSystem;

void registerRenderSystem(sol::state_view state) {
    if (state["RenderSystem"].valid()) return;
    using T = elysian::RenderSystem;
    auto type = registerType<T>(state, "RenderSystem");
}
}
