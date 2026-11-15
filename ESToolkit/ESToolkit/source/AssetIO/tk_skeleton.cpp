/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESToolkit/AssetIO/tk_skeleton.h>
#include <QESTkUtils/tk_menu_stack.h>
#include <ESToolkit/AssetIO/tk_entity.h>
#include <spine/AnimationState.h>
#include <spine/Slot.h>
#include <spine/Skeleton.h>

#include <ESToolkit/ESGamma/tk_lua_wrappers.h>
#include <AssetIO/proxy/elysian_entity_proxy.hpp>
#include <QESTkUtils/ui/delegate/tk_file_browser_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_dynamic_content_property_item_delegate.h>
#include <QESTkUtils/ui/delegate/tk_combobox_property_item_delegate.h>
#include <QESTkUtils/model/tk_lua_table_stack_model.h>
#include <QESTkUtils/tk_menu_stack.h>
#include <QESTkUtils/tk_lua_mime_drop_manager.h>
#include <QStyleOptionViewItem>
#include <QApplication>

//========== SKELETON ==========

void TkSkeleton::onAtlasChanged(void) {
    setAttrib(_attrib);
}

void TkSkeleton::onJsonChanged(void) {
    setAttrib(_attrib);
}

bool TkSkeleton::validate(QString &str) const {
    bool success = true;
    auto cAssert = [&](const bool val, const char* msg) {
        if(!val) {
            success = false;
            str += msg;
            str += "\n";
        }
    };

    cAssert(_attrib.get(), "LOG_ERROR: Skeleton component has no attrib field.");
    return success;
}
void TkSkeleton::setAttrib(std::shared_ptr<SkeletonAttrib> attrib) {
    if(_attrib) _attrib->disconnect(this);

    if(attrib) {
        connect(attrib.get(), SIGNAL(atlasChanged()), this, SLOT(onAtlasChanged()));
        connect(attrib.get(), SIGNAL(jsonChanged()), this, SLOT(onJsonChanged()));
    }

    elysian::SkeletonBase::setAttrib(attrib);
}

QMenu* TkSkeleton::createContextMenu(void) {
    QESTkUtils::MenuStack menuStack("Skeleton");

    menuStack.insertCheckableAction("Attributes",
        [&](bool checked) {
            setAttrib(checked? std::make_shared<SkeletonAttrib>() : nullptr);
        },
        getAttrib().get(),
        !_entity->isActive()
    );

    menuStack.insertAction("Remove", [&](bool /*checked*/) {
            _entity->removeSkeleton();
        },
        canRemoveFrom(*_entity)
    );

    return menuStack.takeTopLevel();

}

void TkSkeleton::initializeDelegates(void) {
    sol::table delegates = elysian::lua::table();
    delegates.set("curAnim", static_cast<PropertyItemDelegate*>(new ComboBoxPropertyItemDelegate([](ESTkStandardItem* item) {
                QStringList options;
                LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                if(luaFieldItem) {
                    TkSkeleton* skeleton = luaFieldItem->getTableRef().as<TkSkeleton*>();
                    if(skeleton && skeleton->_attrib) {
                        for(auto&& it: skeleton->_attrib->_animations) {
                            options.push_back(it->getName());
                        }
                    }
                }
                return options;
            }, true)));
    delegates.set("skin", static_cast<PropertyItemDelegate*>(new ComboBoxPropertyItemDelegate([](ESTkStandardItem* item) {
                QStringList options;
                LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                if(luaFieldItem) {
                    TkSkeleton* skeleton = luaFieldItem->getTableRef().as<TkSkeleton*>();

                    if(skeleton && skeleton->_attrib) {
                        int i = 0;
                        for(;;) {
                            const char* name = skeleton->_attrib->getSpineSkinNameByIndex(i++);
                            if(!name) break;
                            options.push_back(QString(name));
                        }
                    }
                }
                return options;
            }, true)));

    delegates.push();
    lua_setfield(_mainState, -2, TK_LUA_TABLE_DELEGATE_FIELD_KEY);
}

//========== SKELETONATTRIB ==========

void TkSkeletonAttrib::initializeDelegates(void) {
    lua_newtable(_mainState);
    lua_pushvalue(_mainState, -1);
    lua_setfield(_mainState, -3, TK_LUA_TABLE_DELEGATE_FIELD_KEY);

    FileBrowserPropertyItemDelegate* atlasDelegate = new FileBrowserPropertyItemDelegate([](ESTkStandardItem* item, QStyleOptionViewItem& option) {
        LuaTableFieldItem* luaItem = LuaTableFieldItem::fromStandardItem(item);
        if(luaItem) {
        TkSkeletonAttrib* attrib = luaItem->getTableRef().as<TkSkeletonAttrib*>();
            if(attrib) {
                option.text = attrib->getAtlasPath();
                if(!attrib->isAtlasValid()) option.palette.setColor(QPalette::Text, QColor(Qt::red));
            }
        }
    });
    atlasDelegate->addButton(QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)), [](ESTkStandardItem* item) {
        LuaTableFieldItem* luaItem = LuaTableFieldItem::fromStandardItem(item);
        if(luaItem) {
            auto* attrib = luaItem->getTableRef().as<TkSkeletonAttrib*>();
            if(attrib) {
                attrib->reloadAtlas();
            }
        }
    });
    sol::stack::push(_mainState, atlasDelegate);
    lua_setfield(_mainState, -2, "atlasPath");

    FileBrowserPropertyItemDelegate* jsonDelegate = new FileBrowserPropertyItemDelegate([](ESTkStandardItem* item, QStyleOptionViewItem& option) {
        LuaTableFieldItem* luaItem = LuaTableFieldItem::fromStandardItem(item);
        if(luaItem) {
            TkSkeletonAttrib* attrib = luaItem->getTableRef().as<TkSkeletonAttrib*>();
            if(attrib) {
                option.text = attrib->getJsonPath();
                if(!attrib->isJsonValid()) option.palette.setColor(QPalette::Text, QColor(Qt::red));
            }
        }
    });
    jsonDelegate->addButton(QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)), [](ESTkStandardItem* item) {
        LuaTableFieldItem* luaItem = LuaTableFieldItem::fromStandardItem(item);
        if(luaItem) {
            auto* attrib = luaItem->getTableRef().as<TkSkeletonAttrib*>();
            if(attrib) {
                attrib->reloadJson();
            }
        }
    });
    sol::stack::push(_mainState, jsonDelegate);
    lua_setfield(_mainState, -2, "jsonPath");

    DynamicContentPropertyItemDelegate* animDelegate =
            new DynamicContentPropertyItemDelegate(DynamicContentPropertyItemDelegate::luaArraySizeTextCallback<false>);
    animDelegate->addButton(QIcon(DYNAMIC_CONTENT_PROPERTY_ITEM_DELEGATE_ICON_PLUS_PATH), DynamicContentPropertyItemDelegate::luaAddItemButtonCallback<elysian::Animation>);
    animDelegate->setMimeDropManager(new LuaMimeDropManager(QMap<QString, LuaMimeDropManager::LuaHandler> {
        {
            QString("Entity"),
            {
                [](sol::object v, Qt::DropAction, QDropEvent*, int, int, const QModelIndex& parent) {
                    auto* model = qobject_cast<const QStandardItemModel*>(parent.model());
                    if(model) {
                        LuaTableFieldItem* luaItem = LuaTableFieldItem::fromStandardItem(model->itemFromIndex(parent));
                        if(luaItem) {
                            const sol::object& self = luaItem->getTableData()->getTableRef();
                            sol::object srcTable = elysian::lua::get(elysian::lua::get(elysian::lua::get(v, "skeleton"), "attrib"), "animations");

                            for(std::size_t i = 1; i <= elysian::lua::length(srcTable); ++i) {
                                elysian::lua::set(self, i, elysian::lua::get(srcTable, i));
                            }

                            return true;
                        }
                    }
                    return false;
                },
                [](sol::object v, Qt::DropAction, QDragEnterEvent*, int, int, const QModelIndex&) {
                    return !!elysian::lua::get(elysian::lua::get(elysian::lua::get(v, "skeleton"), "attrib"), "animations").valid();
                }
            }
        }

    }));

    sol::stack::push(_mainState, animDelegate);
    lua_setfield(_mainState, -2, "animations");

    lua_pop(_mainState, 1);
}

void TkSkeletonAttrib::initializeVirtualTableDelegates(QString key) {
    if(key == "animations") {

        auto delegateFunc = [](sol::object t, sol::object k) -> PropertyItemDelegate* {
            TkSkeletonAttrib* self = elysian::lua::get(t, "__ud").as<TkSkeletonAttrib*>();
            if(self && elysian::lua::type(k) == LUA_TNUMBER) {
                unsigned index = k.as<unsigned>();
                if(index <= self->getAnimationCount()) {

                    DynamicContentPropertyItemDelegate* delegate = new DynamicContentPropertyItemDelegate([](ESTkStandardItem* item, QStyleOptionViewItem& option) {
                        LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                        if(luaFieldItem) {
                            TkSkeletonAttrib* attrib = elysian::lua::get(luaFieldItem->getTableRef(), "__ud").as<TkSkeletonAttrib*>();
                            if(attrib) {
                                unsigned index = luaFieldItem->getKey().as<unsigned>()-1;
                                elysian::Animation* anim = attrib->getAnimation(index);
                                if(anim) {
                                    option.palette.setColor(QPalette::Text, QColor(Qt::gray));
                                    option.text = QString(anim->getName());
                                }
                            }
                        }
                    });

                    delegate->addButton(QApplication::style()->standardIcon(QStyle::SP_MediaPlay), [](ESTkStandardItem* item) {
                        auto* luaItem = LuaTableFieldItem::fromStandardItem(item);
                        if(luaItem) {
                            auto* skeletonItem = luaItem->getAncestorSiblingLuaItem({"animations", "attrib", "skeleton"}, 1);
                            if(skeletonItem) {
                                elysian::lua::set(skeletonItem->getVariant(), "curAnim", elysian::lua::get(luaItem->getVariant(), "name"));
                            }
                        }
                    });

                    delegate->setDeleteCallback([](ESTkStandardItem* item) {
                        LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                        if(luaFieldItem) {
                            TkSkeletonAttrib* attrib = elysian::lua::get(luaFieldItem->getTableRef(), "__ud").as<TkSkeletonAttrib*>();
                            if(attrib) {
                                unsigned index = luaFieldItem->getKey().as<unsigned>()-1;
                                attrib->setAnimation(index, nullptr);

                                auto* attribItem = luaFieldItem->getAncestorSiblingLuaItem({"animations", "attrib"}, 1);
                                if(attribItem) {
                                    attribItem->setDirty();
                                }
                            }
                        }

                    });

                    delegate->setContextMenuCallback([](ESTkStandardItem* item) -> QMenu* {
                        LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                        if(luaFieldItem) {
                            TkSkeletonAttrib* attrib = elysian::lua::get(luaFieldItem->getTableRef(), "__ud").as<TkSkeletonAttrib*>();
                            if(attrib) {
                                unsigned index = luaFieldItem->getKey().as<unsigned>()-1;

                                QESTkUtils::MenuStack stack("Animation");
                                stack.insertAction("Delete", [=](bool) {
                                    attrib->setAnimation(index, nullptr);

                                    auto* attribItem = luaFieldItem->getAncestorSiblingLuaItem({"animations", "attrib"}, 1);
                                    if(attribItem) {
                                        attribItem->setDirty();
                                    }

                                });

                                return stack.takeTopLevel();
                            }
                        }
                        return nullptr;
                    });

                    return delegate;
                }
            }
            return nullptr;
        };

        sol::stack::push(_mainState, sol::as_function(delegateFunc));
        lua_setfield(_mainState, -2, TK_LUA_TABLE_DELEGATE_FIELD_KEY);
    }
}


//========== ANIMATION ==========


void TkAnimation::initializeDelegates(void) {
    lua_newtable(_mainState);
    lua_pushvalue(_mainState, -1);
    lua_setfield(_mainState, -3, TK_LUA_TABLE_DELEGATE_FIELD_KEY);

    auto* nameDelegate = new PropertyItemDelegate();
    nameDelegate->setCompleterCallback([](ESTkStandardItem*) {
        QStringList list;
        list << "Idle";
        sol::object nameTable = LuaManager::exec("return Character.animNameList");
        for(std::size_t i = 1; i < elysian::lua::length(nameTable); ++i) {
            QString name = elysian::lua::get(nameTable, i).as<const char*>();
            list << name+QString("Left") << name+QString("Right") << name+QString("Up") << name+QString("Down");
        }
        return list;
    }, QCompleter::UnfilteredPopupCompletion);
    sol::stack::push(_mainState, nameDelegate);
    lua_setfield(_mainState, -2, "name");

    DynamicContentPropertyItemDelegate* tracksDelegate =
            new DynamicContentPropertyItemDelegate(DynamicContentPropertyItemDelegate::luaArraySizeTextCallback<false>);
    tracksDelegate->addButton(QIcon(DYNAMIC_CONTENT_PROPERTY_ITEM_DELEGATE_ICON_PLUS_PATH), DynamicContentPropertyItemDelegate::luaAddItemButtonCallback<AnimationTrack>);

    sol::stack::push(_mainState, tracksDelegate);
    lua_setfield(_mainState, -2, "tracks");

    lua_pop(_mainState, 1);
}

void TkAnimation::initializeVirtualTableDelegates(QString key) {
    if(key == "tracks") {

        auto delegateFunc = [](sol::object t, sol::object k) -> PropertyItemDelegate* {
            TkAnimation* self = elysian::lua::get(t, "__ud").as<TkAnimation*>();
            if(self && elysian::lua::type(k) == LUA_TNUMBER) {
                unsigned index = k.as<unsigned>();
                if(index <= self->getTrackCount()) {
                    DynamicContentPropertyItemDelegate* delegate = new DynamicContentPropertyItemDelegate([](ESTkStandardItem* item, QStyleOptionViewItem& option) {
                        LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                        if(luaFieldItem) {
                            TkAnimation* anim = elysian::lua::get(luaFieldItem->getTableRef(), "__ud").as<TkAnimation*>();
                            if(anim) {
                                unsigned index = luaFieldItem->getKey().as<unsigned>()-1;
                                elysian::AnimationTrack* track = anim->getTrack(index);
                                option.palette.setColor(QPalette::Text, QColor(Qt::gray));
                                option.text = QString(track->getName());
                            }
                        }
                    });

                    delegate->setContextMenuCallback([](ESTkStandardItem* item) -> QMenu* {
                        LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                        if(luaFieldItem) {
                            TkAnimation* anim = elysian::lua::get(luaFieldItem->getTableRef(), "__ud").as<TkAnimation*>();
                            if(anim) {
                                unsigned index = luaFieldItem->getKey().as<unsigned>()-1;

                                QESTkUtils::MenuStack stack("AnimationTrack");
                                stack.insertAction("Delete", [=](bool) {
                                    anim->setTrack(index, nullptr);

                                    auto* parentItem = luaFieldItem->getAncestorSiblingLuaItem({"tracks", "*"}, 1);
                                    if(parentItem) parentItem->setDirty();

                                });

                                return stack.takeTopLevel();
                            }
                        }
                        return nullptr;

                    });

                    delegate->setDeleteCallback([](ESTkStandardItem* item) {
                        LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
                        if(luaFieldItem) {
                            TkAnimation* anim = elysian::lua::get(luaFieldItem->getTableRef(), "__ud").as<TkAnimation*>();
                            if(anim) {
                                unsigned index = luaFieldItem->getKey().as<unsigned>()-1;
                                anim->setTrack(index, nullptr);

                                auto* parentItem = luaFieldItem->getAncestorSiblingLuaItem({"tracks", "*"}, 1);
                                if(parentItem) parentItem->setDirty();
                            }
                        }
                    });

                    return delegate;
                }
            }
            return nullptr;
        };
        sol::stack::push(_mainState, sol::as_function(delegateFunc));
        lua_setfield(_mainState, -2, TK_LUA_TABLE_DELEGATE_FIELD_KEY);
    }
}


//========== ANIMATIONTRACK ==========

void TkAnimationTrack::initializeDelegates(void) {
    ComboBoxPropertyItemDelegate* nameDelegate = new ComboBoxPropertyItemDelegate([](ESTkStandardItem* item) {
            QStringList options;
            LuaTableFieldItem* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
            if(luaFieldItem) {
                LuaTableFieldItem* attribItem = luaFieldItem->getAncestorSiblingLuaItem({"*", "tracks", "*", "animations", "attrib"}, 1);
                TkSkeletonAttrib* attrib = attribItem->getVariant().as<TkSkeletonAttrib*>();
                if(attrib) {
                    int i = 0;
                    for(;;) {
                        const char* name = attrib->getSpineAnimNameByIndex(i++);
                        if(!name) break;
                        options.push_back(QString(name));
                    }
                }
            }
            return options;
        }, true);

    nameDelegate->setTextCallback([](ESTkStandardItem* item, QStyleOptionViewItem& option) {
        auto* luaFieldItem = LuaTableFieldItem::fromStandardItem(item);
        if(luaFieldItem) {
            LuaTableFieldItem* attribItem = luaFieldItem->getAncestorSiblingLuaItem({"*", "tracks", "*", "animations", "attrib"}, 1);
            if(attribItem) {
                TkSkeletonAttrib* attrib = attribItem->getVariant().as<TkSkeletonAttrib*>();
                if(attrib) {
                    option.text = item->data(Qt::EditRole).toString();
                    if(!attrib->findSpineAnimation(Q_CSTR(option.text))) option.palette.setColor(QPalette::Text, QColor(Qt::red));
                }
            }
        }
    });


    sol::table delegates = elysian::lua::table();
    delegates.set("name", static_cast<PropertyItemDelegate*>(nameDelegate));

    delegates.push();
    lua_setfield(_mainState, -2, TK_LUA_TABLE_DELEGATE_FIELD_KEY);
}


void TkSkeleton::printSlotData(void) const {
    DbgEvent event("Entity[%s].skeleton.printSlotData()", _entity->getName());

    if(_attrib) {
        for(int i = 0; i < _skeleton->slotsCount; ++i) {
            dbgprintf("slot[%d]: %s", i, _skeleton->data->slots[i]->name);
            dbgpush();

            auto* attachment = _skeleton->slots[i]->attachment;
            if(attachment) {
                bool found = false;

                for(unsigned i = 0; i < _entityAttachments.size(); ++i) {
                    if(attachment == _entityAttachments[i]) {
                        dbgprintf("attachment: %s (ENTITY)", _entityAttachments[i]->entity->getName());
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    dbgprintf("attachment: %s (SPINE)", attachment->name);
                }
            } else {
                dbgprintf("attachment: NONE");
            }

            dbgpop();
        }
    } else {
        warnprintf("Skeleton component has no attributes!");
    }
}

TkSkeleton::~TkSkeleton(void) {
    if(_attrib.get()) {
        dbgprintf("Deleting SkeletonAttrib: %d references remaining!", _attrib.use_count()-1);
    }
}
