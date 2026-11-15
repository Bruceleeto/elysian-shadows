/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef TEST_DIALOG_H
#define TEST_DIALOG_H

#include <QtTest>
#include <QDebug>
#include <ESToolkit/AssetIO/tk_dialog.h>
#include <AssetIO/utility/elysian_lua_values.hpp>
#include <AssetIO/proxy/elysian_entity_proxy.hpp>
#include <AssetIO/proxy/elysian_dialog_proxy.hpp>

namespace tktest {

class DialogLuaTest: public QObject, public LuaManager {
    Q_OBJECT

private Q_SLOTS:

    //void initTestCase(void);

    //void createNew(void);
    //void createFromReference(void);

    //void create(void);
    void screenTextProperty(void);
    //void screenTriggerProperty(void);
    //void screenSourceProperty(void);
};


inline void DialogLuaTest::screenTextProperty(void) {
    DialogText text;
    auto object = elysian::lua::make(&text);
    elysian::lua::call(elysian::lua::get(object, "setText"), object, "Dialog text");
    QCOMPARE(elysian::lua::get(object, "text").as<std::string>(), std::string("Dialog text"));
    elysian::lua::set(object, "text", "Updated text");
    QCOMPARE(elysian::lua::call(elysian::lua::get(object, "getText"), object).as<std::string>(), std::string("Updated text"));
}

}

#endif // TEST_DIALOG_H

