/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>
#include <ESGamma/proxy/elysian_layout_proxy.hpp>
#include <ESGamma/proxy/elysian_widget_proxy.hpp>

/*!
    \qmltype Widget
    \since 1.6.0
    \inmodule Engine
    \brief Abstract base class for all visible UI elements.

    A Widget is the lowest-level base class used to represent any object that will be
    displayed and/or interacted with from the UI. These can be text labels, sprites,
    skeletal animations, buttons, dropdown menus, etc. They can also recursively contain
    child widgets, which can then have their own children as well. For example, something
    like a list of items could be represented as a single \l{GroupWidget} containing multiple
    \l {LabelWidget}s as its children.

    The overall feel and design of the Widget system is inspired by \l{http://doc.qt.io/qt-5/qtwidgets-index.html}{The Qt Widgets Module},
    but there are some differences present to make it more suited to the Elysian Shadows
    Engine and game development in general. These are most obvious in how \l{Event}s are
    handled, and how the \l{EventSystem} is used instead of Qt signals/slots.

    \section2 Handling Input
    Due to the wide array of platforms supported by the Elysian Shadows Engine, it's
    important to consider the various forms of input which the user may use to interact
    with widgets and menus when targeting multiple platforms. There are usually two
    disjoint methods of menu interaction that need to be accounted for to support all
    input mechanisms.

    \section3 Controller and Keyboard Input
    The most common form of user input is most likely going to be via \l Controller on
    consoles or \l Keyboard on PC (and some consoles). Input from these devices should
    be handled in the same manner as with \l{LuaBehavior}s. One thing to keep in mind
    is the concept of \e modality, as a parent menu with submenus will usually only
    care about input for lowest-level active submenu. Imagine a GroupWidget with
    multiple ListWidget children. Only one of the children can be active at a time,
    otherwise pressing the arrow keys can cause both menus to scroll through selections.

    \e {NOTE: It's very much advised that in the case of \l Controller Widget navigation,
    the \l VirtualDPad class is used rather than the direct dpad buttons or analog sticks,
    so that you can handle both of them uniformly rather than only supporting one or the
    other in the menus.}

    \section3 Mouse and Touch Input
    Other common input mechanisms are either a \l Mouse (PC, some consoles) or \l{TouchDevice}s
    (usually smartphones or tablets). These two input mechanisms differ from Controller and
    Keyboard input in that they usually do not require the concept of widget modality, and instead
    allow you to click on any widget anywhere, rather than caring which child widget(s) are currently
    acive.

    \section2 Layouts
    In order to add any subwidgets to a widget, it must have a corresponding Layout.
    \l{Layout} are object used to group child Widgets and dictate how they should
    be organized.

    \section2 Widget Types
    As Widget is an abstract base-class, you will not be instantiating it directly.
    Instead you will be instantiating one fo the following child types:
    \table
        \header
            \li \b {Class}
            \li \b {Description}
        \row
            \li EntityWidget
            \li Generic widget allowing for any Entity to be rendered as a UI-component
        \row
            \li LabelWidget
            \li Widget used to display text
        \row
            \li GroupWidget
            \li Simple, blank parent widget simply used to organize child widgets
     \endtable

    \sa EntityWidget, LabelWidget, GroupWidget, Layout, GuiSystem
*/

/*!
    \qmltype EntityWidget
    \inherits Widget
    \since 1.6.0
    \inmodule Engine
    \brief Generic Widget for using an Entity as a UI Element

    EntityWidget is a lightweight container that simply uses an existing Entity object
    as a UI element, supporting all of its regular graphical features--\l Renderable and
    \l Skeleton components. This allows you to fully customize the aesthetic of any UI
    element and can come in handy in a party menu, for example, when you want to render
    existing character Entities in a UI menu.
*/

/*!
    \qmltype LabelWidget
    \since 1.6.0
    \inmodule Engine
    \inherits Widget
    \brief Widget used for displaying text in a GUI

    LabelWidget is a lightweight Widget used for displaying generic text as a UI element.
    Color can be controlled manually or via the standard Font UI markup in the text string.
*/

/*!
    \qmltype GroupWidget
    \since 1.6.0
    \inmodule Engine
    \inherits Widget
    \brief Simple blank Widget for holding other Widgets

    GroupWidget is a lightweight Widget used to simply hold other widgets.
    Favor this as opposed to creating a full-fledged custom-widget or an
    \l EntityWidget when the parent widget won't even be rendered and doesn't
    need anything other than standard layout logic.
*/



namespace elysian::lua {
using namespace elysian;
using Widget = elysian::Widget;
using GroupWidget = elysian::GroupWidget;
using EntityWidget = elysian::EntityWidget;
using LabelWidget = elysian::LabelWidget;

void registerWidget(sol::state_view state) {
    if (state["Widget"].valid()) return;
    using T = elysian::Widget;
    auto type = registerType<T>(state, "Widget");
    type["getPosition"] = method<T, Vector2() const>(&T::getPosition);
    type["setPosition"] = method<T, void(Vector2)>(&T::setPosition);
    type["getSize"] = method<T, Vector2() const>(&T::getSize);
    type["setSize"] = method<T, void(Vector2)>(&T::setSize);
    type["setMaxSize"] = method<T, void(Vector2)>(&T::setMaxSize);
    type["setMinSize"] = method<T, void(Vector2)>(&T::setMinSize);
    type["getMarginSize"] = method<T, Vector2()>(&T::getMarginSize);
    type["setMarginSize"] = method<T, void(Vector2)>(&T::setMarginSize);
    type["setHorizontalSizePolicy"] = method<T, void(uint16_t)>(&T::setHorizontalSizePolicy);
    type["setVerticalSizePolicy"] = method<T, void(uint16_t)>(&T::setVerticalSizePolicy);
    type["setSizePolicy"] = method<T, void(uint16_t, uint16_t)>(&T::setSizePolicy);
    type["setLayout"] = method<T, void(TableLayout*)>(&T::setLayout);
    type["setEnabled"] = method<T, void(bool)>(&T::setEnabled);
    type["getHorizontalSizePolicy"] = method<T, uint16_t() const>(&T::getHorizontalSizePolicy);
    type["getVerticalSizePolicy"] = method<T, uint16_t() const>(&T::getVerticalSizePolicy);
    type["getLayout"] = method<T, TableLayout*() const>(&T::getLayout<TableLayout>);
    type["isEnabled"] = method<T, bool() const>(&T::isEnabled);
}

void registerGroupWidget(sol::state_view state) {
    if (state["GroupWidget"].valid()) return;
    registerWidget(state);
    using T = elysian::GroupWidget;
    auto type = registerType<T, elysian::Widget>(state, "GroupWidget");
    type["new"] = sol::factories(factory<T>());
}

void registerEntityWidget(sol::state_view state) {
    if (state["EntityWidget"].valid()) return;
    registerWidget(state);
    using T = elysian::EntityWidget;
    auto type = registerType<T, elysian::Widget>(state, "EntityWidget");
    type["new"] = sol::factories(factory<T>(), factory<T, Entity*>());
    type["setEntity"] = method<T, void(Entity*)>(&T::setEntity);
    type["getEntity"] = method<T, Entity*() const>(&T::getEntity);
}

void registerLabelWidget(sol::state_view state) {
    if (state["LabelWidget"].valid()) return;
    registerWidget(state);
    using T = elysian::LabelWidget;
    auto type = registerType<T, elysian::Widget>(state, "LabelWidget");
    type["new"] = sol::factories(factory<T>(), factory<T, const char*>());
    type["setText"] = method<T, void(const char*)>(&T::setText);
    type["setColor"] = method<T, void(Color4)>(&T::setColor);
    type["getText"] = method<T, const char*() const>(&T::getText);
    type["getColor"] = method<T, Color4() const>(&T::getColor);
}
}
