/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>
#include <ESGamma/proxy/elysian_layout_proxy.hpp>
#include <ESGamma/proxy/elysian_widget_proxy.hpp>

/*!
    \qmltype TableLayout
    \since 1.6.0
    \inmodule Engine
    \brief Abstract class for arranging Widgets as child GUI elements within a parent Widget.
*/

/*!
    \qmltype SingleAxisLayout
    \inherits TableLayout
    \since 1.6.0
    \inmodule Engine
    \brief Abstract class for TableLayouts whose size is limited to only a single dimension
*/
/*!
    \qmltype HorizontalLayout
    \since 1.6.0
    \inmodule Engine
    \brief GUI Layout for arranging Widgets horizontally into columns within a parent Widget.
*/

/*!
    \qmltype VerticalLayout
    \since 1.6.0
    \inmodule Engine
    \brief GUI Layout for arranging Widgets vertically into rows within a parent Widget.
*/


namespace elysian::lua {
using namespace elysian;
using TableLayout = elysian::TableLayout;
using SingleAxisLayout = elysian::SingleAxisLayout;
using HorizontalLayout = elysian::HorizontalLayout;
using VerticalLayout = elysian::VerticalLayout;

void registerTableLayout(sol::state_view state) {
    if (state["TableLayout"].valid()) return;
    using T = elysian::TableLayout;
    auto type = registerType<T>(state, "TableLayout");
    type["removeWidget"] = method<T, Widget*(Widget*)>(&T::removeWidget);
    type["getWidgetCount"] = method<T, unsigned() const>(&T::getWidgetCount);
    type["getParent"] = method<T, Widget*() const>(&T::getParent);
    type["setHorizontalSpacing"] = method<T, void(int)>(&T::setHorizontalSpacing);
    type["getHorizontalSpacing"] = method<T, int() const>(&T::getHorizontalSpacing);
    type["setVerticalSpacing"] = method<T, void(int)>(&T::setVerticalSpacing);
    type["getVerticalSpacing"] = method<T, int() const>(&T::getVerticalSpacing);
    type["setSpacing"] = method<T, void(int, int)>(&T::setSpacing);
    type["getHorizontalAlignment"] = method<T, TableLayout::ALIGNMENT() const>(&T::getHorizontalAlignment);
    type["setHorizontalAlignment"] = method<T, void(TableLayout::ALIGNMENT)>(&T::setHorizontalAlignment);
    type["getVerticalAlignment"] = method<T, TableLayout::ALIGNMENT() const>(&T::getVerticalAlignment);
    type["setVerticalAlignment"] = method<T, void(TableLayout::ALIGNMENT)>(&T::setVerticalAlignment);
    type["setAlignment"] = method<T, void(TableLayout::ALIGNMENT, TableLayout::ALIGNMENT)>(&T::setAlignment);
}

void registerSingleAxisLayout(sol::state_view state) {
    if (state["SingleAxisLayout"].valid()) return;
    registerTableLayout(state);
    using T = elysian::SingleAxisLayout;
    auto type = registerType<T, elysian::TableLayout>(state, "SingleAxisLayout");
    type["addWidget"] = [](T& self, sol::object arg0) { return std::invoke(method<T, void(Widget*)>(&T::addWidget), self, take<Widget>(arg0)); };
    type["getWidget"] = method<T, Widget*(unsigned) const>(&T::getWidget);
    type["setWidgets"] = method<T, void(sol::object)>(&T::setWidgetsLua);
    type["getWidgets"] = method<T, sol::object() const>(&T::getWidgetsLua);
}

void registerHorizontalLayout(sol::state_view state) {
    if (state["HorizontalLayout"].valid()) return;
    registerSingleAxisLayout(state);
    registerTableLayout(state);
    using T = elysian::HorizontalLayout;
    auto type = registerType<T, elysian::SingleAxisLayout, elysian::TableLayout>(state, "HorizontalLayout");
    type["new"] = sol::factories(factory<T>());
}

void registerVerticalLayout(sol::state_view state) {
    if (state["VerticalLayout"].valid()) return;
    registerSingleAxisLayout(state);
    registerTableLayout(state);
    using T = elysian::VerticalLayout;
    auto type = registerType<T, elysian::SingleAxisLayout, elysian::TableLayout>(state, "VerticalLayout");
    type["new"] = sol::factories(factory<T>());
}
}
