/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef TK_LUAWRAPPERS_H
#define TK_LUAWRAPPERS_H

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <QMessageBox>
#include <AssetIO/crtp/elysian_color.hpp>
#include <ESGamma/lua/elysian_lua_singleton.hpp>
#include <ESToolkit/ui/tk_estoolkit.h>
#include <ESToolkit/AssetIO/tk_area.h>
#include <QESTkUtils/ui/delegate/tk_file_browser_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_spinbox_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_enum_dropdown_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_combobox_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_dynamic_content_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_color_swatch_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_plain_text_edit_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_slider_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_property_item_delegate.h>



class Toolkit {};



























static inline void luaRegisterTkEnumsConstants(sol::state_view /*state*/) {

}








namespace elysian::lua {
void registerToolkit(sol::state_view state);
void registerPropertyItemDelegate(sol::state_view state);
void registerEnumDropdownPropertyItemDelegate(sol::state_view state);
void registerSpinboxPropertyItemDelegate(sol::state_view state);
void registerFileBrowserPropertyItemDelegate(sol::state_view state);
void registerComboBoxPropertyItemDelegate(sol::state_view state);
void registerDynamicContentPropertyItemDelegate(sol::state_view state);
void registerQESTkUtils__ColorSwatchPropertyItemDelegate_Color4_(sol::state_view state);
void registerQESTkUtils__ColorSwatchPropertyItemDelegate_Vector4_(sol::state_view state);
void registerQESTkUtils__PlainTextEditPropertyItemDelegate(sol::state_view state);
void registerQESTkUtils__SliderPropertyItemDelegate(sol::state_view state);
}

#endif // TK_LUAWRAPPERS_H
