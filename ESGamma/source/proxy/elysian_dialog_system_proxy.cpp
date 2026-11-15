/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>
#include <ESGamma/proxy/elysian_dialog_system_proxy.hpp>

/*!
    \qmltype DialogSystem
    \since 1.5.8
    \inmodule Engine
    \brief Engine subsystem handling dialog-related logic.

    The DialogSystem is the \l{Engine} subsystem for handling dialog, text descriptions,
    and anything else involving the dialog box.

    \section2 Retrieving the DialogSystem
    The DialogSystem object can be retrieved through the static \l{Engine} class:
    \code
    dialogSys = Engine.getDialogSys()
    \endcode

    \section2 Prompting for User Input with Lua
    In general, \l{Dialog} is created through the ESTk GUI and is attached to an \l{Entity}
    through its \l{Examinable} \l{Component}. Upon inspection, the dialog starts
    automatically. For the full summary of configuring Dialog through ESTk, see the
    \l{Dialog Trees} page for more details.

    \section2 Starting a Dialog Directly through Lua
    Once a \l{Dialog} object and its \l{DialogScreen}s have been properly configured
    (see \l{Dialog Trees}), simply starting a dialog with the object can be done in
    a single line of Lua code:
    \code
    Engine.getDialogSys():startDialog(dialogObject, EventMonitor.null())
    \endcode

    \sa Dialog, Examinable, Engine
*/

/*!
 * \qmlmethod void DialogSystem::startDialog(Dialog* dialog, EventMonitor* monitor)
 * Opens the dialog box and begins the given \a{dialog}, notifying the \l{EventMonitor},
 * \a{monitor}, when the dialog is complete.
 * \sa EventMonitor
 */


namespace elysian::lua {
using namespace elysian;
using DialogSystem = elysian::DialogSystem;

void registerDialogSystem(sol::state_view state) {
    if (state["DialogSystem"].valid()) return;
    using T = elysian::DialogSystem;
    auto type = registerType<T>(state, "DialogSystem");
    type["startDialog"] = [](T& self, std::shared_ptr<Dialog> arg0, sol::object arg1) { return std::invoke(method<T, void(std::shared_ptr<Dialog>, EventMonitor*)>(&T::startDialog), self, arg0, take<EventMonitor>(arg1)); };
}
}
