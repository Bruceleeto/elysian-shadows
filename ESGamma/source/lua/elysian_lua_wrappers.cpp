/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_shader_api.h>

#include <AssetIO/level/elysian_map_depth_buffer.hpp>
#include <AssetIO/proxy/elysian_entity_proxy.hpp>
#include <AssetIO/proxy/elysian_level_proxy.hpp>
#include <AssetIO/proxy/elysian_area_proxy.hpp>
#include <AssetIO/proxy/elysian_bit_mask_proxy.hpp>
#include <AssetIO/proxy/elysian_event_proxy.hpp>

#include <ESGamma/gui/elysian_layout.hpp>
#include <ESGamma/proxy/elysian_input_system_proxy.hpp>
#include <ESGamma/proxy/elysian_widget_proxy.hpp>
#include <ESGamma/proxy/elysian_layout_proxy.hpp>
#include <ESGamma/proxy/elysian_gui_system_proxy.hpp>
#include <ESGamma/proxy/elysian_level_system_proxy.hpp>
#include <ESGamma/proxy/elysian_character_system_proxy.hpp>
#include <ESGamma/proxy/elysian_audio_system_proxy.hpp>
#include <ESGamma/proxy/elysian_camera_system_proxy.hpp>
#include <ESGamma/proxy/elysian_combat_system_proxy.hpp>
#include <ESGamma/proxy/elysian_day_system_proxy.hpp>
#include <ESGamma/proxy/elysian_dialog_system_proxy.hpp>
#include <ESGamma/proxy/elysian_light_system_proxy.hpp>
#include <ESGamma/proxy/elysian_particle_system_proxy.hpp>
#include <ESGamma/proxy/elysian_party_system_proxy.hpp>
#include <ESGamma/proxy/elysian_rigid_body_system_proxy.hpp>
#include <ESGamma/proxy/elysian_timer_system_proxy.hpp>
#include <ESGamma/proxy/elysian_event_system_proxy.hpp>
#include <ESGamma/proxy/elysian_collision_system_proxy.hpp>
#include <ESGamma/proxy/elysian_examinable_system_proxy.hpp>
#include <ESGamma/proxy/elysian_render_system_proxy.hpp>
#include <ESGamma/proxy/elysian_thread_system_proxy.hpp>
#include <ESGamma/crtp/elysian_lua_manager.hpp>
#include <ESGamma/lua/elysian_lua_wrappers.hpp>

#ifdef ELYSIAN_INPUT_SYSTEM_SENSORS_ENABLED
#   include <ESGamma/device/elysian_sensor_device.hpp>
#endif

namespace elysian {


    // ====== C/++ ENUMERATIONS AND CONSTANTS WRAPPERS ======
    static void luaRegisterEnumsConstants(sol::state_view s) {

        //gyro::TYPES
        s["gyro"]["COLLIDER_NONE"] = GY_COLLIDER_NONE;
        s["gyro"]["COLLIDER_AABB"] = GY_COLLIDER_AABB;
        s["gyro"]["COLLIDER_OBB"] = GY_COLLIDER_OBB;
        s["gyro"]["COLLIDER_RTRI"] = GY_COLLIDER_RTRI;
        s["gyro"]["COLLIDER_CIRCLE"] = GY_COLLIDER_CIR;
        s["gyro"]["TEX_ORIENT_NORMAL"] = GY_RECT_TEX_ORIENT_NORMAL;
        s["gyro"]["TEX_ORIENT_FLIP_HORIZ"] = GY_RECT_TEX_ORIENT_FLIP_HORIZ;
        s["gyro"]["TEX_ORIENT_FLIP_VERT"] = GY_RECT_TEX_ORIENT_FLIP_VERT;
        s["gyro"]["TEX_ORIENT_ROT_90"] = GY_RECT_TEX_ORIENT_ROT_90;
        s["gyro"]["TEX_ORIENT_ROT_180"] = GY_RECT_TEX_ORIENT_ROT_180;
        s["gyro"]["TEX_ORIENT_ROT_270"] = GY_RECT_TEX_ORIENT_ROT_270;
        s["gyro"]["TEX_ORIENT_FLIP_HORIZ_ROT_90"] = GY_RECT_TEX_ORIENT_FLIP_HORIZ_ROT_90;
        s["gyro"]["TEX_ORIENT_FLIP_HORIZ_ROT_270"] = GY_RECT_TEX_ORIENT_FLIP_HORIZ_ROT_270;

        s["gyro"]["LIGHT_DIRECTIONAL"] = GY_LIGHT_DIRECTIONAL;
        s["gyro"]["LIGHT_SPOT"] = GY_LIGHT_SPOT;
        s["gyro"]["LIGHT_POINT"] = GY_LIGHT_POINT;

        s["gyro"]["RECT_PLANE_BOTTOM"] = GY_RECT_PLANE_BOTTOM;
        s["gyro"]["RECT_PLANE_FRONT"] = GY_RECT_PLANE_FRONT;
        s["gyro"]["RECT_PLANE_LEFT"] = GY_RECT_PLANE_LEFT;
        s["gyro"]["RECT_PLANE_RIGHT"] = GY_RECT_PLANE_RIGHT;
        s["gyro"]["RECT_PLANE_FRONT_SLOPE_45"] = GY_RECT_PLANE_FRONT_SLOPE_45;
        s["gyro"]["RECT_PLANE_LEFT_SLOPE_45"] = GY_RECT_PLANE_LEFT_SLOPE_45;
        s["gyro"]["RECT_PLANE_RIGHT_SLOPE_45"] = GY_RECT_PLANE_RIGHT_SLOPE_45;

        //BlendMode
        s["BlendMode"]["BLEND_ZERO"] = GY_BLEND_ZERO;
        s["BlendMode"]["BLEND_ONE"] = GY_BLEND_ONE;
        s["BlendMode"]["BLEND_SRC_COLOR"] = GY_BLEND_SRC_COLOR;
        s["BlendMode"]["BLEND_ONE_MINUS_SRC_COLOR"] = GY_BLEND_ONE_MINUS_SRC_COLOR;
        s["BlendMode"]["BLEND_DST_COLOR"] = GY_BLEND_DST_COLOR;
        s["BlendMode"]["BLEND_ONE_MINUS_DST_COLOR"] = GY_BLEND_ONE_MINUS_DST_COLOR;
        s["BlendMode"]["BLEND_SRC_ALPHA"] = GY_BLEND_SRC_ALPHA;
        s["BlendMode"]["BLEND_ONE_MINUS_SRC_ALPHA"] = GY_BLEND_ONE_MINUS_SRC_ALPHA;
        s["BlendMode"]["BLEND_DST_ALPHA"] = GY_BLEND_DST_ALPHA;
        s["BlendMode"]["BLEND_ONE_MINUS_DST_ALPHA"] = GY_BLEND_ONE_MINUS_DST_ALPHA;
        s["BlendMode"]["BLEND_CONSTANT_COLOR"] = GY_BLEND_CONSTANT_COLOR;
        s["BlendMode"]["BLEND_ONE_MINUS_CONSTANT_COLOR"] = GY_BLEND_ONE_MINUS_CONSTANT_COLOR;
        s["BlendMode"]["BLEND_CONSTANT_ALPHA"] = GY_BLEND_CONSTANT_ALPHA;
        s["BlendMode"]["BLEND_ONE_MINUS_CONSTANT_ALPHA"] = GY_BLEND_ONE_MINUS_CONSTANT_ALPHA;
        s["BlendMode"]["BLEND_SRC_ALPHA_SATURATE"] = GY_BLEND_SRC_ALPHA_SATURATE;
        s["BlendMode"]["BLEND_SRC1_COLOR"] = GY_BLEND_SRC1_COLOR;
        s["BlendMode"]["BLEND_ONE_MINUS_SRC1_COLOR"] = GY_BLEND_ONE_MINUS_SRC1_COLOR;
        s["BlendMode"]["BLEND_SRC1_ALPHA"] = GY_BLEND_SRC1_ALPHA;
        s["BlendMode"]["BLEND_ONE_MINUS_SRC1_ALPHA"] = GY_BLEND_ONE_MINUS_SRC1_ALPHA;

        //Entity::DEPTH_TYPE
        s["Entity"]["NORMAL_FLAT"] = elysian::NORMAL_FLAT;
        s["Entity"]["NORMAL_FRONT"] = elysian::NORMAL_FRONT;

        //CollisionFlags::TYPE
        typedef elysian::CollisionFlags CollisionFlags;
        s["CollisionFlags"]["TILE"] = CollisionFlags::TILE;
        s["CollisionFlags"]["RENDERABLE"] = CollisionFlags::RENDERABLE;
        s["CollisionFlags"]["COLLIDABLE"] = CollisionFlags::COLLIDABLE;
        s["CollisionFlags"]["CHARACTER"] = CollisionFlags::CHARACTER;
        s["CollisionFlags"]["WARP"] = CollisionFlags::WARP;
        s["CollisionFlags"]["RIGID_BODY"] = CollisionFlags::RIGID_BODY;
        s["CollisionFlags"]["ITEM"] = CollisionFlags::ITEM;
        s["CollisionFlags"]["FRIENDLY"] = CollisionFlags::FRIENDLY;
        s["CollisionFlags"]["ENEMY"] = CollisionFlags::ENEMY;
        s["CollisionFlags"]["PLAYER"] = CollisionFlags::PLAYER;
        s["CollisionFlags"]["ALL"] = CollisionFlags::ALL;

        //Combat::DMG_TYPE
        typedef elysian::Combat Combat;
        s["Combat"]["DMG_NONE"] = Combat::DMG_NONE;
        s["Combat"]["DMG_SOURCE"] = Combat::DMG_SOURCE;
        s["Combat"]["DMG_TARGET"] = Combat::DMG_TARGET;
        s["Combat"]["DMG_SOURCE_TARGET"] = Combat::DMG_SOURCE_TARGET;

        s["Combat"]["TARGET_NONE"] = Combat::TARGET_NONE;
        s["Combat"]["TARGET_PARTY"] = Combat::TARGET_PARTY;
        s["Combat"]["TARGET_FRIENDLY"] = Combat::TARGET_FRIENDLY;
        s["Combat"]["TARGET_ENEMY"] = Combat::TARGET_ENEMY;

        s["Combat"]["ON_COLLIDE"] = Combat::ON_COLLIDE;
        s["Combat"]["ON_COLLISION_ENTER"] = Combat::ON_COLLISION_ENTER;
        s["Combat"]["ON_COLLISION_EXIT"] = Combat::ON_COLLISION_EXIT;

        //CombatStatus::STATUS_EFFECT
        typedef elysian::CombatStatus CombatStatus;
        s["CombatStatus"]["POISON"] = CombatStatus::POISON;
        s["CombatStatus"]["CONFUSION"] = CombatStatus::CONFUSION;
        s["CombatStatus"]["SLEEP"] = CombatStatus::SLEEP;

        //CombatSystem::COMBAT_STATE
        typedef elysian::CombatSystem CombatSystem;
        s["CombatSystem"]["COMBAT_STATE_DISABLED"] = CombatSystem::COMBAT_STATE_DISABLED;
        s["CombatSystem"]["COMBAT_STATE_DISABLED"] = CombatSystem::COMBAT_STATE_INACTIVE;
        s["CombatSystem"]["COMBAT_STATE_DISABLED"] = CombatSystem::COMBAT_STATE_DISABLED;

        //CombatSystem::COMBAT_MGMT_TYPE
        s["CombatSystem"]["COMBAT_MGMT_AUTO"] = CombatSystem::COMBAT_MGMT_AUTO;
        s["CombatSystem"]["COMBAT_MGMT_MANUAL"] = CombatSystem::COMBAT_MGMT_MANUAL;

        //Behavior::TRIGGER
        typedef elysian::Behavior Behavior;
        s["Behavior"]["ON_SCRIPT_CREATE"] = Behavior::ON_SCRIPT_CREATE;
        s["Behavior"]["ON_SCRIPT_DESTROY"] = Behavior::ON_SCRIPT_DESTROY;
        s["Behavior"]["ON_CREATE"] = Behavior::ON_CREATE;
        s["Behavior"]["ON_DESTROY"] = Behavior::ON_DESTROY;
        s["Behavior"]["ON_LOAD"] = Behavior::ON_LOAD;
        s["Behavior"]["ON_UNLOAD"] = Behavior::ON_UNLOAD;
        s["Behavior"]["ON_ENTER"] = Behavior::ON_ENTER;
        s["Behavior"]["ON_EXIT"] = Behavior::ON_EXIT;
        s["Behavior"]["ON_UPDATE"] = Behavior::ON_UPDATE;
        s["Behavior"]["ON_RENDER"] = Behavior::ON_RENDER;
        s["Behavior"]["ON_COLLIDE"] = Behavior::ON_COLLIDE;
        s["Behavior"]["ON_COLLISION_ENTER"] = Behavior::ON_COLLISION_ENTER;
        s["Behavior"]["ON_COLLISION_EXIT"] = Behavior::ON_COLLISION_EXIT;
        s["Behavior"]["ON_EXAMINE"] = Behavior::ON_EXAMINE;
        s["Behavior"]["ON_DIALOG"] = Behavior::ON_DIALOG;
        s["Behavior"]["ON_COMBAT_ENTER"] = Behavior::ON_COMBAT_ENTER;
        s["Behavior"]["ON_COMBAT_EXIT"] = Behavior::ON_COMBAT_EXIT;
        s["Behavior"]["ON_COMBAT_DEATH"] = Behavior::ON_COMBAT_DEATH;
        s["Behavior"]["ON_SHATTER"] = Behavior::ON_SHATTER;
        s["Behavior"]["ON_COMBAT_RECEIVE_STATUS_MODIFIER"] = Behavior::ON_COMBAT_RECEIVE_STATUS_MODIFIER;
        s["Behavior"]["ON_COMBAT_TARGET_COLLISION_ENTER"] = Behavior::ON_COMBAT_TARGET_COLLISION_ENTER;
        s["Behavior"]["ON_BUTTON_PRESS"] = Behavior::ON_BUTTON_PRESS;
        s["Behavior"]["ON_BUTTON_RELEASE"] = Behavior::ON_BUTTON_RELEASE;
        s["Behavior"]["ON_TK_SELECT"] = Behavior::ON_TK_SELECT;
        s["Behavior"]["ON_TK_UNSELECT"] = Behavior::ON_TK_UNSELECT;

        s["Behavior"]["ON_SCRIPT_CREATE_INDEX"] = Behavior::ON_SCRIPT_CREATE_INDEX;
        s["Behavior"]["ON_SCRIPT_DESTROY_INDEX"] = Behavior::ON_SCRIPT_DESTROY_INDEX;
        s["Behavior"]["ON_CREATE_INDEX"] = Behavior::ON_CREATE_INDEX;
        s["Behavior"]["ON_DESTROY_INDEX"] = Behavior::ON_DESTROY_INDEX;
        s["Behavior"]["ON_LOAD_INDEX"] = Behavior::ON_LOAD_INDEX;
        s["Behavior"]["ON_UNLOAD_INDEX"] = Behavior::ON_UNLOAD_INDEX;
        s["Behavior"]["ON_ENTER_INDEX"] = Behavior::ON_ENTER_INDEX;
        s["Behavior"]["ON_EXIT_INDEX"] = Behavior::ON_EXIT_INDEX;
        s["Behavior"]["ON_UPDATE_INDEX"] = Behavior::ON_UPDATE_INDEX;
        s["Behavior"]["ON_RENDER_INDEX"] = Behavior::ON_RENDER_INDEX;
        s["Behavior"]["ON_COLLIDE_INDEX"] = Behavior::ON_COLLIDE_INDEX;
        s["Behavior"]["ON_COLLISION_ENTER_INDEX"] = Behavior::ON_COLLISION_ENTER_INDEX;
        s["Behavior"]["ON_COLLISION_EXIT_INDEX"] = Behavior::ON_COLLISION_EXIT_INDEX;
        s["Behavior"]["ON_EXAMINE_INDEX"] = Behavior::ON_EXAMINE_INDEX;
        s["Behavior"]["ON_DIALOG_INDEX"] = Behavior::ON_DIALOG_INDEX;
        s["Behavior"]["ON_COMBAT_ENTER_INDEX"] = Behavior::ON_COMBAT_ENTER_INDEX;
        s["Behavior"]["ON_COMBAT_EXIT_INDEX"] = Behavior::ON_COMBAT_EXIT_INDEX;
        s["Behavior"]["ON_COMBAT_DEATH_INDEX"] = Behavior::ON_COMBAT_DEATH_INDEX;
        s["Behavior"]["ON_SHATTER_INDEX"] = Behavior::ON_SHATTER_INDEX;
        s["Behavior"]["ON_COMBAT_RECEIVE_STATUS_MODIFIER_INDEX"] = Behavior::ON_COMBAT_RECEIVE_STATUS_MODIFIER_INDEX;
        s["Behavior"]["ON_COMBAT_TARGET_COLLISION_ENTER_INDEX"] = Behavior::ON_COMBAT_TARGET_COLLISION_ENTER_INDEX;
        s["Behavior"]["ON_BUTTON_PRESS_INDEX"] = Behavior::ON_BUTTON_PRESS_INDEX;
        s["Behavior"]["ON_BUTTON_RELEASE_INDEX"] = Behavior::ON_BUTTON_RELEASE_INDEX;
        s["Behavior"]["ON_EVENT_RECEIVE"] = Behavior::ON_RECEIVE_EVENT;
        s["Behavior"]["ON_EVENT_OBSERVE"] = Behavior::ON_OBSERVE_EVENT;
        s["Behavior"]["ON_TK_SELECT_INDEX"] = Behavior::ON_TK_SELECT_INDEX;
        s["Behavior"]["ON_TK_UNSELECT_INDEX"] = Behavior::ON_TK_UNSELECT_INDEX;
        s["Behavior"]["TRIGGER_MAX_INDEX"] = Behavior::TRIGGER_MAX_INDEX;

        //Tile::COLLIDER_TYPE
        typedef elysian::Tile Tile;
        s["Tile"]["COLLIDER_NONE"] = Tile::COLLIDER_NONE;
        s["Tile"]["COLLIDER_FULL"] = Tile::COLLIDER_FULL;
        s["Tile"]["COLLIDER_HALF_E"] = Tile::COLLIDER_HALF_E;
        s["Tile"]["COLLIDER_HALF_N"] = Tile::COLLIDER_HALF_N;
        s["Tile"]["COLLIDER_HALF_W"] = Tile::COLLIDER_HALF_W;
        s["Tile"]["COLLIDER_HALF_S"] = Tile::COLLIDER_HALF_S;
        s["Tile"]["COLLIDER_QUAD_SE"] = Tile::COLLIDER_QUAD_SE;
        s["Tile"]["COLLIDER_QUAD_NE"] = Tile::COLLIDER_QUAD_NE;
        s["Tile"]["COLLIDER_QUAD_NW"] = Tile::COLLIDER_QUAD_NW;
        s["Tile"]["COLLIDER_QUAD_SW"] = Tile::COLLIDER_QUAD_SW;
        s["Tile"]["COLLIDER_L_SE"] = Tile::COLLIDER_L_SE;
        s["Tile"]["COLLIDER_L_NE"] = Tile::COLLIDER_L_NE;
        s["Tile"]["COLLIDER_L_NW"] = Tile::COLLIDER_L_NW;
        s["Tile"]["COLLIDER_L_SW"] = Tile::COLLIDER_L_SW;
        s["Tile"]["COLLIDER_QUADS_SE_NW"] = Tile::COLLIDER_QUADS_SE_NW;
        s["Tile"]["COLLIDER_QUADS_SW_NE"] = Tile::COLLIDER_QUADS_SW_NE;
        s["Tile"]["COLLIDER_45DEG_SE"] = Tile::COLLIDER_45DEG_SE;
        s["Tile"]["COLLIDER_45DEG_NE"] = Tile::COLLIDER_45DEG_NE;
        s["Tile"]["COLLIDER_45DEG_NW"] = Tile::COLLIDER_45DEG_NW;
        s["Tile"]["COLLIDER_45DEG_SW"] = Tile::COLLIDER_45DEG_SW;

        //Keyboard::KEYS
        s["Keyboard"]["KEY_NONE"] = GY_KEY_NONE;
        s["Keyboard"]["KEY_A"] = GY_KEY_A;
        s["Keyboard"]["KEY_B"] = GY_KEY_B;
        s["Keyboard"]["KEY_C"] = GY_KEY_C;
        s["Keyboard"]["KEY_D"] = GY_KEY_D;
        s["Keyboard"]["KEY_E"] = GY_KEY_E;
        s["Keyboard"]["KEY_F"] = GY_KEY_F;
        s["Keyboard"]["KEY_G"] = GY_KEY_G;
        s["Keyboard"]["KEY_H"] = GY_KEY_H;
        s["Keyboard"]["KEY_I"] = GY_KEY_I;
        s["Keyboard"]["KEY_J"] = GY_KEY_J;
        s["Keyboard"]["KEY_K"] = GY_KEY_K;
        s["Keyboard"]["KEY_L"] = GY_KEY_L;
        s["Keyboard"]["KEY_M"] = GY_KEY_M;
        s["Keyboard"]["KEY_N"] = GY_KEY_N;
        s["Keyboard"]["KEY_O"] = GY_KEY_O;
        s["Keyboard"]["KEY_P"] = GY_KEY_P;
        s["Keyboard"]["KEY_Q"] = GY_KEY_Q;
        s["Keyboard"]["KEY_R"] = GY_KEY_R;
        s["Keyboard"]["KEY_S"] = GY_KEY_S;
        s["Keyboard"]["KEY_T"] = GY_KEY_T;
        s["Keyboard"]["KEY_U"] = GY_KEY_U;
        s["Keyboard"]["KEY_V"] = GY_KEY_V;
        s["Keyboard"]["KEY_W"] = GY_KEY_W;
        s["Keyboard"]["KEY_X"] = GY_KEY_X;
        s["Keyboard"]["KEY_Y"] = GY_KEY_Y;
        s["Keyboard"]["KEY_Z"] = GY_KEY_Z;
        s["Keyboard"]["KEY_DPAD_U"] = GY_KEY_DPAD_U;
        s["Keyboard"]["KEY_DPAD_D"] = GY_KEY_DPAD_D;
        s["Keyboard"]["KEY_DPAD_L"] = GY_KEY_DPAD_L;
        s["Keyboard"]["KEY_DPAD_R"] = GY_KEY_DPAD_R;
        s["Keyboard"]["KEY_SPACE"] = GY_KEY_SPACE;
        s["Keyboard"]["KEY_RETURN"] = GY_KEY_RETURN;
        s["Keyboard"]["KEY_ESC"] = GY_KEY_ESC;
        s["Keyboard"]["KEY_BACKSLASH"] = GY_KEY_BACKSLASH;
        s["Keyboard"]["KEY_FORWARDSLASH"] = GY_KEY_FORWARDSLASH;
        s["Keyboard"]["KEY_LSHIFT"] = GY_KEY_LSHIFT;
        s["Keyboard"]["KEY_RSHIFT"] = GY_KEY_RSHIFT;
        s["Keyboard"]["KEY_LCTRL"] = GY_KEY_LCTRL;
        s["Keyboard"]["KEY_RCTRL"] = GY_KEY_RCTRL;
        s["Keyboard"]["KEY_LALT"] = GY_KEY_LALT;
        s["Keyboard"]["KEY_RALT"] = GY_KEY_RALT;
        s["Keyboard"]["KEY_BACKQUOTE"] = GY_KEY_BACKQUOTE;
        s["Keyboard"]["KEY_COUNT"] = GY_KEY_COUNT;

        s["Controller"]["BUT_DPAD_UP"] = GY_CONTROLLER_BUT_DPAD_UP;
        s["Controller"]["BUT_DPAD_DOWN"] = GY_CONTROLLER_BUT_DPAD_DOWN;
        s["Controller"]["BUT_DPAD_LEFT"] = GY_CONTROLLER_BUT_DPAD_LEFT;
        s["Controller"]["BUT_DPAD_RIGHT"] = GY_CONTROLLER_BUT_DPAD_RIGHT;
        s["Controller"]["BUT_START"] = GY_CONTROLLER_BUT_START;
        s["Controller"]["BUT_SELECT"] = GY_CONTROLLER_BUT_SELECT;
        s["Controller"]["BUT_HOME"] = GY_CONTROLLER_BUT_HOME;
        s["Controller"]["BUT_LSHOULDER"] = GY_CONTROLLER_BUT_LSHOULDER;
        s["Controller"]["BUT_RSHOULDER"] = GY_CONTROLLER_BUT_RSHOULDER;
        s["Controller"]["BUT_LANALOG"] = GY_CONTROLLER_BUT_LANALOG;
        s["Controller"]["BUT_RANALOG"] = GY_CONTROLLER_BUT_RANALOG;
        s["Controller"]["BUT_X"] = GY_CONTROLLER_BUT_X;
        s["Controller"]["BUT_SQUARE"] = GY_CONTROLLER_BUT_SQUARE;
        s["Controller"]["BUT_CIRCLE"] = GY_CONTROLLER_BUT_CIRCLE;
        s["Controller"]["BUT_TRIANGLE"] = GY_CONTROLLER_BUT_TRIANGLE;
        s["Controller"]["BUT_COUNT"] = GY_CONTROLLER_BUT_COUNT;

        s["Controller"]["ANALOG_LEFT"] = GY_CONTROLLER_ANALOG_LEFT;
        s["Controller"]["ANALOG_RIGHT"] = GY_CONTROLLER_ANALOG_RIGHT;

        s["Controller"]["TRIGGER_LEFT"] = GY_CONTROLLER_TRIGGER_LEFT;
        s["Controller"]["TRIGGER_RIGHT"] = GY_CONTROLLER_TRIGGER_RIGHT;

        s["Mouse"]["BUT_NONE"] = GY_MOUSE_BUT_NONE;
        s["Mouse"]["BUT_LEFT"] = GY_MOUSE_BUT_LEFT;
        s["Mouse"]["BUT_MIDDLE"] = GY_MOUSE_BUT_MIDDLE;
        s["Mouse"]["BUT_RIGHT"] = GY_MOUSE_BUT_RIGHT;
        s["Mouse"]["BUT_BACK"] = GY_MOUSE_BUT_BACK;

        s["InputSystem"]["BUT_STATE_UP"] = GY_BUT_STATE_UP;
        s["InputSystem"]["BUT_STATE_TAPPED"] = GY_BUT_STATE_TAPPED;
        s["InputSystem"]["BUT_STATE_RELEASED"] = GY_BUT_STATE_RELEASED;
        s["InputSystem"]["BUT_STATE_DOWN"] = GY_BUT_STATE_DOWN;

        //Direction::DIR4
        typedef elysian::Direction Direction;
        s["Direction"]["DIR4_CENTER"] = Direction::DIR4_CENTER;
        s["Direction"]["DIR4_LEFT"] = Direction::DIR4_LEFT;
        s["Direction"]["DIR4_RIGHT"] = Direction::DIR4_RIGHT;
        s["Direction"]["DIR4_UP"] = Direction::DIR4_UP;
        s["Direction"]["DIR4_DOWN"] = Direction::DIR4_DOWN;

        //Character::ANIM_STATE
        typedef elysian::Character Character;
        s["Character"]["STATE_STAND"] = Character::STATE_STAND;
        s["Character"]["STATE_WALK"] = Character::STATE_WALK;
        s["Character"]["STATE_RUN"] = Character::STATE_RUN;
        s["Character"]["STATE_DASH"] = Character::STATE_DASH;
        s["Character"]["STATE_JUMP"] = Character::STATE_JUMP;
        s["Character"]["STATE_FALL"] = Character::STATE_FALL;
        s["Character"]["STATE_PUSH"] = Character::STATE_PUSH;
        s["Character"]["STATE_DRAG"] = Character::STATE_DRAG;
        s["Character"]["STATE_HOLD"] = Character::STATE_HOLD;
        s["Character"]["STATE_THROW"] = Character::STATE_THROW;
        s["Character"]["STATE_COMBAT"] = Character::STATE_COMBAT;
        s["Character"]["STATE_STRAFE"] = Character::STATE_STRAFE;
        s["Character"]["STATE_KNOCKBACK"] = Character::STATE_KNOCKBACK;
        s["Character"]["STATE_DEATH"] = Character::STATE_DEATH;
        s["Character"]["STATE_LUA"] = Character::STATE_LUA;

        s["RumbleEffect"]["INFINITE"] = GY_RUMBLE_EFFECT_DURATION_INFINITE;

        s["PeriodicRumbleEffect"]["SINE"] = GY_RUMBLE_EFFECT_PERIODIC_SINE;
        s["PeriodicRumbleEffect"]["TRIANGLE"] = GY_RUMBLE_EFFECT_PERIODIC_TRIANGLE;
        s["PeriodicRumbleEffect"]["SAWTOOTHUP"] = GY_RUMBLE_EFFECT_PERIODIC_SAWTOOTHUP;
        s["PeriodicRumbleEffect"]["SAWTOOTHDOWN"] = GY_RUMBLE_EFFECT_PERIODIC_SAWTOOTHDOWN;

        typedef elysian::TableLayout TableLayout;
        s["TableLayout"]["ALIGN_NEGATIVE"] = TableLayout::ALIGN_NEGATIVE;
        s["TableLayout"]["ALIGN_CENTER"] = TableLayout::ALIGN_CENTER;
        s["TableLayout"]["ALIGN_POSITIVE"] = TableLayout::ALIGN_POSITIVE;

        typedef elysian::PartySystem PartySystem;
        s["PartySystem"]["UnassignedIndex"] = PartySystem::UnassignedPlayerIndex;
        s["PartySystem"]["InvalidIndex"] = PartySystem::InvalidPlayerIndex;

        s["EventTarget"]["INVALID"] = EVENT_TARGET_INVALID;
        s["EventTarget"]["ENTITY"] = EVENT_TARGET_ENTITY;
        s["EventTarget"]["BEHAVIOR"] = EVENT_TARGET_BEHAVIOR;
        s["EventTarget"]["WIDGET"] = EVENT_TARGET_WIDGET;
        s["EventTarget"]["LUA"] = EVENT_TARGET_LUA;

        s["EventTarget"]["STATE_PENDING"] = EVENT_STATE_PENDING;
        s["EventTarget"]["STATE_ACCEPTED"] = EVENT_STATE_ACCEPTED;
        s["EventTarget"]["STATE_IGNORED"] = EVENT_STATE_IGNORED;

        s["Event"]["ENTITY_ACTIVATE"] = EVENT_ENTITY_ACTIVATE;
        s["Event"]["ENTITY_DEACTIVATE"] = EVENT_ENTITY_DEACTIVATE;
        s["Event"]["KEYBOARD_ATTACHED"] = EVENT_KEYBOARD_ATTACHED;
        s["Event"]["KEYBOARD_REMOVED"] = EVENT_KEYBOARD_REMOVED;
        s["Event"]["KEYBOARD_KEY_PRESSED"] = EVENT_KEYBOARD_KEY_PRESSED;
        s["Event"]["KEYBOARD_KEY_RELEASED"] = EVENT_KEYBOARD_KEY_RELEASED;
        s["Event"]["MOUSE_ATTACHED"] = EVENT_MOUSE_ATTACHED;
        s["Event"]["MOUSE_REMOVED"] = EVENT_MOUSE_REMOVED;
        s["Event"]["MOUSE_MOVED"] = EVENT_MOUSE_MOVED;
        s["Event"]["MOUSE_BUTTON_PRESSED"] = EVENT_MOUSE_BUTTON_PRESSED;
        s["Event"]["MOUSE_BUTTON_RELEASED"] = EVENT_MOUSE_BUTTON_RELEASED;
        s["Event"]["MOUSE_SCROLLED"] = EVENT_MOUSE_SCROLLED;
        s["Event"]["CONTROLLER_ATTACHED"] = EVENT_CONTROLLER_ATTACHED;
        s["Event"]["CONTROLLER_REMOVED"] = EVENT_CONTROLLER_REMOVED;
        s["Event"]["CONTROLLER_BUTTON_PRESSED"] = EVENT_CONTROLLER_BUTTON_PRESSED;
        s["Event"]["CONTROLLER_BUTTON_RELEASED"] = EVENT_CONTROLLER_BUTTON_RELEASED;
        s["Event"]["CONTROLLER_ANALOG_MOVED"] = EVENT_CONTROLLER_ANALOG_MOVED;
        s["Event"]["CONTROLLER_TRIGGER_MOVED"] = EVENT_CONTROLLER_TRIGGER_MOVED;
        s["Event"]["RUMBLE_DEVICE_ATTACHED"] = EVENT_RUMBLE_DEVICE_ATTACHED;
        s["Event"]["RUMBLE_DEVICE_REMOVED"] = EVENT_RUMBLE_DEVICE_REMOVED;
        s["Event"]["VMU_ATTACHED"] = EVENT_VMU_ATTACHED;
        s["Event"]["VMU_REMOVED"] = EVENT_VMU_REMOVED;
        s["Event"]["COLLISION_ENTER"] = EVENT_COLLISION_ENTER;
        s["Event"]["COLLISION_EXIT"] = EVENT_COLLISION_EXIT;
        s["Event"]["AREA_ENTER"] = EVENT_AREA_ENTER;
        s["Event"]["AREA_EXIT"] = EVENT_AREA_EXIT;
        s["Event"]["AREA_CHANGED"] = EVENT_AREA_CHANGED;
        s["Event"]["GAME_TIME_DAWN"] = EVENT_GAME_TIME_DAWN;
        s["Event"]["GAME_TIME_NOON"] = EVENT_GAME_TIME_NOON;
        s["Event"]["GAME_TIME_DUSK"] = EVENT_GAME_TIME_DUSK;
        s["Event"]["GAME_TIME_MIDNIGHT"] = EVENT_GAME_TIME_MIDNIGHT;
        s["Event"]["LUA"] = EVENT_LUA;

    }


    void LuaManagerBase::_registerWrappers(void) {
        //Register classes and structs that have been proxied to Lua
        elysian::lua::registerRecti(_mainState);
        elysian::lua::registerBitMask32(_mainState);
        elysian::lua::registerBitMask64(_mainState);
		elysian::lua::registerLuaBehaviorScriptAttrib(_mainState);
        elysian::lua::registerGYVector2(_mainState);
        elysian::lua::registerGYVector3(_mainState);
        elysian::lua::registerGYVector4(_mainState);
        elysian::lua::registerVector2(_mainState);
        elysian::lua::registerVector3(_mainState);
        elysian::lua::registerVector4(_mainState);
        elysian::lua::registerQuaternion(_mainState);
        elysian::lua::registerGYColor4(_mainState);
        elysian::lua::registerColor4(_mainState);
		elysian::lua::registerLuaEventCallback(_mainState);
		elysian::lua::registerInstanceTransform(_mainState);
		elysian::lua::registerEntity(_mainState);
        elysian::lua::registerBlendMode(_mainState);
        elysian::lua::registerSpriteAttrib(_mainState);
        elysian::lua::registerQuadSpriteAttrib(_mainState);
		elysian::lua::registerComponent(_mainState);
		elysian::lua::registerMaterial(_mainState);
		elysian::lua::registerRenderable(_mainState);
        elysian::lua::registerLightAttrib(_mainState);
		elysian::lua::registerCollisionFlags(_mainState);
        elysian::lua::registerCollidableStaticAttrib(_mainState);
		elysian::lua::registerCollidableAttrib(_mainState);
		elysian::lua::registerCollisionContact(_mainState);
		elysian::lua::registerCollidable(_mainState);
		elysian::lua::registerCharacter(_mainState);
		elysian::lua::registerWarp(_mainState);
		elysian::lua::registerDialogScreen(_mainState);
		elysian::lua::registerDialogText(_mainState);
        elysian::lua::registerDialogOption(_mainState);
		elysian::lua::registerDialogPrompt(_mainState);
		elysian::lua::registerDialog(_mainState);
		elysian::lua::registerExaminable(_mainState);
        elysian::lua::registerCombatStatusModifier(_mainState);
		elysian::lua::registerCombatAttrib(_mainState);
        elysian::lua::registerAggroManager(_mainState);
		elysian::lua::registerCombat(_mainState);
        elysian::lua::registerRigidBodyAttrib(_mainState);
        elysian::lua::registerForceTarget(_mainState);
        elysian::lua::registerRigidBody(_mainState);
		elysian::lua::registerParticleEmitterAttrib(_mainState);
		elysian::lua::registerParticleEmitter(_mainState);

        elysian::lua::registerEvent(_mainState);
        elysian::lua::registerLuaEvent(_mainState);
        elysian::lua::registerKeyEvent(_mainState);
        elysian::lua::registerControllerEvent(_mainState);
        elysian::lua::registerControllerButtonEvent(_mainState);
        elysian::lua::registerControllerAnalogEvent(_mainState);
        elysian::lua::registerControllerTriggerEvent(_mainState);
        elysian::lua::registerMouseBaseEvent(_mainState);
        elysian::lua::registerMouseEvent(_mainState);
        elysian::lua::registerMouseWheelEvent(_mainState);

        elysian::lua::registerEventTarget(_mainState);
        elysian::lua::registerEventObserver(_mainState);
        elysian::lua::registerEventSystem(_mainState);
        elysian::lua::registerLuaBehaviorScript(_mainState);
		elysian::lua::registerLuaBehavior(_mainState);
		elysian::lua::registerTile(_mainState);
		elysian::lua::registerLevel(_mainState);
		elysian::lua::registerTileEntry(_mainState);
		elysian::lua::registerArea(_mainState);
    //	elysian::lua::registerGameInput(_mainState);
       // elysian::lua::registerMouseInput(_mainState);
		elysian::lua::registerPartySystem(_mainState);
		elysian::lua::registerTimerSystem(_mainState);
        elysian::lua::registerEngine(_mainState);
		elysian::lua::registergyro(_mainState);
		elysian::lua::registerGYWav(_mainState);
		elysian::lua::registerAudioSource(_mainState);
		elysian::lua::registerLightSource(_mainState);
		elysian::lua::registerCharacterSystem(_mainState);
		elysian::lua::registerAudioSystem(_mainState);
		elysian::lua::registerLightSystem(_mainState);
		elysian::lua::registerParticleSystem(_mainState);
		elysian::lua::registerDaySystem(_mainState);
        elysian::lua::registerExaminableSystem(_mainState);
		elysian::lua::registerDialogSystem(_mainState);
        elysian::lua::registerCollisionSystem(_mainState);
		elysian::lua::registerRigidBodySystem(_mainState);
        elysian::lua::registerThreadSystem(_mainState);
        elysian::lua::registerRenderSystem(_mainState);
        elysian::lua::registerAdventureLog(_mainState);
        elysian::lua::registerHorizontalLayout(_mainState);
        elysian::lua::registerVerticalLayout(_mainState);
        elysian::lua::registerEntityWidget(_mainState);
        elysian::lua::registerLabelWidget(_mainState);
		elysian::lua::registerGuiSystem(_mainState);
		elysian::lua::registerLevelSystem(_mainState);
        elysian::lua::registerController(_mainState);
        elysian::lua::registerMouse(_mainState);
        elysian::lua::registerTouchDevice(_mainState);
        elysian::lua::registerRumbleEffect(_mainState);
        elysian::lua::registerDirectionalRumbleEffect(_mainState);
        elysian::lua::registerLeftRightRumbleEffect(_mainState);
        elysian::lua::registerConstantRumbleEffect(_mainState);
        elysian::lua::registerRampRumbleEffect(_mainState);
        elysian::lua::registerPeriodicRumbleEffect(_mainState);
        elysian::lua::registerRumbleDevice(_mainState);
        elysian::lua::registerVisualMemoryUnit(_mainState);
        elysian::lua::registerKeyboard(_mainState);
#ifdef ELYSIAN_INPUT_SYSTEM_SENSORS_ENABLED
        elysian::lua::registerAccelerometer(_mainState);
        elysian::lua::registerGyroscope(_mainState);
        elysian::lua::registerMagnetometer(_mainState);
        elysian::lua::registerAltimeter(_mainState);
        elysian::lua::registerPedometer(_mainState);
        elysian::lua::registerScale(_mainState);
#endif
        elysian::lua::registerInputSystem(_mainState);
		elysian::lua::registerCombatSystem(_mainState);
		elysian::lua::registerCameraSystem(_mainState);
		elysian::lua::registerDirection(_mainState);
		elysian::lua::registerGravityForceGenerator(_mainState);
		elysian::lua::registerImplosionForceGenerator(_mainState);
		elysian::lua::registerExplosionForceGenerator(_mainState);
		elysian::lua::registerCameraKeyFrame(_mainState);
        elysian::lua::registerAnimationTrack(_mainState);
        elysian::lua::registerAnimation(_mainState);
        elysian::lua::registerSkeletonAttrib(_mainState);
        elysian::lua::registerSkeleton(_mainState);


		//Register C and static member functions
		lua_register(_mainState, "print", &print);
        lua_register(_mainState, "warnprint", &warnprint);
        lua_register(_mainState, "errprint", &errprint);
        lua_register(_mainState, "printAdvLog", &printAdvLog);
        lua_register(_mainState, "dbgPush", &dbgPush);
        lua_register(_mainState, "dbgPop", &dbgPop);
        _mainState["Engine"]["getCurArea"] = &Singletons_getCurArea;
        _mainState["Engine"]["getCurLevel"] = &Singletons_getCurLevel;
        _mainState["Engine"]["setProcessInput"] = &Singletons_setProcessInput;
        _mainState["Engine"]["getPartySys"] = &Singletons_getPartySystem;
        _mainState["Engine"]["getInputSys"] = &Singletons_getInputSystem;
        _mainState["Engine"]["getTimerSys"] = &Singletons_getTimerSystem;
        _mainState["Engine"]["getAudioSys"] = &Singletons_getAudioSystem;
        _mainState["Engine"]["getLightSys"] = &Singletons_getLightSystem;
        _mainState["Engine"]["getLevelSys"] = &Singletons_getLevelSystem;
        _mainState["Engine"]["getCombatSys"] = &Singletons_getCombatSystem;
        _mainState["Engine"]["getPartSys"] = &Singletons_getParticleSystem;
        _mainState["Engine"]["getDaySys"] = &Singletons_getDaySystem;
        _mainState["Engine"]["getCollisionSys"] = &Singletons_getCollisionSystem;
        _mainState["Engine"]["getRigidBodySys"] = &Singletons_getRigidBodySystem;
        _mainState["Engine"]["getDialogSys"] = &Singletons_getDialogSystem;
        _mainState["Engine"]["getGuiSys"] = &Singletons_getGuiSystem;
        _mainState["Engine"]["getCameraSys"] = &Singletons_getCameraSystem;
        _mainState["Engine"]["getCharSys"] = &Singletons_getCharacterSystem;
        _mainState["Engine"]["getEventSys"] = &Singletons_getEventSystem;
        _mainState["Engine"]["getExamSys"] = &Singletons_getExaminableSystem;
        _mainState["Engine"]["getRenderSys"] = &Singletons_getRenderSystem;
        _mainState["Engine"]["getThreadSys"] = &Singletons_getThreadSystem;

        _mainState["InputSystem"]["importControllerButtonMappings"] = &InputSystem_importControllerButtonMappings;

		_mainState["EventMonitor"]["null"] = &EventMonitor_null;

        _mainState["Direction"]["fromVector2"] = &Direction_fromVector2;
        _mainState["Direction"]["toVector2"] = &Direction_toVector2;
        _mainState["Direction"]["toVector3"] = &Direction_toVector3;

		//libGyro wrappers

		_mainState["gyro"]["createWav"] = &gyroWavCreate;
		_mainState["gyro"]["destroyWav"] = &gyroWavDestroy;

		//Register constants and enumerations
		luaRegisterEnumsConstants(_mainState);
	}

}

/*!
    \qmltype Engine
    \since 1.6.0
    \inmodule Engine
    \brief Static class for Engine providing access to all subsystems.

    Engine is a static class (class of all static functions) providing access
    to the various subsystems and the current \l{Level} and \l{Area}.

    \section2 Asset/Data Access and Centrality
    90% of what is operated upon by the engine and its various subsystems is
    stored within the current \l{Level} and \a{Area} objects, rather than within
    the subsystems themselves. This drastically simplifies the process of loading
    and swapping between multiple levels and areas, and allows for the ones that
    are not active to be cached to a certain extent for platforms with enough RAM
    to do so. The main advantage is that Entities, components, and other data does
    not have to be copied in and out of each subsystem when an Area is activated or
    when new data arrives. Since it's all accessed indirectly through the current
    area, swapping areas (and levels) ultimately becomes simply reassigning a
    pointer internally, and the higher-level game logic and Lua scripts never
    have to worry about maintaining consistency between assets in a Level/Area
    and the subsystems or even data stored within one subsystem vs. another.

    From a Behavior and LuaBehavior scripting perspective, most of the data you
    need to access and manipulate from within your scripts are also stored within
    either the current Level or Area. You can retreive these through the Engine.
    \code
    local area = Engine.getCurArea()
    local level = Engine.getCurLevel()
    \endcode

    \section2 Subsystems
    The engine itself delegates all processing, input, and rendering logic to a
    series of subsystems, most of which manage a particular type of \l{Component}.
    \table
        \header
            \li \b{Subsystem}
            \li \b{Managed Data}
            \li \b{Functionality}
        \row
            \li \l{AudioSystem}
            \li \l{AudioSource}
            \li Music and SFX playback
        \row
            \li \l{CameraSystem}
            \li \l{CameraKeyFrame}
            \li Camera properties, cutscenes, boundaries, viewport
        \row
            \li \l{CharacterSystem}
            \li \l{Character}
            \li Basic character functionality: walking, running, jumping, holding, etc
        \row
            \li \l{CollisionSystem}
            \li \l{Collidable}
            \li Collision checks, contact generation, penetration resolution
        \row
            \li \l{CombatSystem}
            \li \l{Combat}
            \li Battles, stats, aggro, damage resolution, death
        \row
            \li \l{DaySystem}
            \li Sun \l{Entity}
            \li Day/Night dynamic lighting, in-game time
        \row
            \li \l{DialogSystem}
            \li \l{Dialog}
            \li Dialog box text from conversations, examined items, messages
        \row
            \li \l{EventSystem}
            \li \l{Event}
            \li Sends, receives, and propogates events to other objects
        \row
            \li \l{ExaminableSystem}
            \li \l{Examinable}
            \li Object/Entity inspection/interaction
        \row
            \li \l{GuiSystem}
            \li \l{Widget}
            \li Menus, UIs, overlays
        \row
            \li \l{InputSystem}
            \li \l{PlayerIODevice}
            \li \l{Controller}, \l{Keyboard}, \l{Mouse}, \l{VisualMemoryUnit}
        \row
            \li \l{LevelSystem}
            \li \l{Level}, \l{Area}
            \li Maintains current level/area, and handles swapping between new ones
        \row
            \li \l{LightSystem}
            \li \l{LightSource}
            \li Handles all of dynamic lighting
        \row
            \li \l{ParticleSystem}
            \li \l{ParticleEmitter}
            \li Emitting, updating, and maintaining particles
        \row
            \li \l{PartySystem}
            \li \l{Playable}
            \li Basic player input like walking, running, jumping, throwing, etc
        \row
            \li \l{RigidBodySystem}
            \li \l{RigidBody}, \l{ForceGenerator}
            \li All of physics integration, force application, contact velocity resolution
        \row
            \li \l{TimerSystem}
            \li \l{TimerSystem::getDeltaTime()}{deltaTime}
            \li Calculates frame time, maintains framerate, manages timers
        \row
            \li \l{ThreadSystem}
            \li Threads, ThreadTasks
            \li Manages CPU threads and thread-level parallelization
        \row
            \li \l{RenderSystem}
            \li \l{Renderable}, \l{Skeleton}, \l{LightSource}
            \li Handles all rendering and displaying graphics
    \endtable

    \sa PartySystem, InputSystem, TimerSystem, AudioSystem, LightSystem, LevelSystem,
    CombatSystem, ParticleSystem, DaySystem, CollisionSystem, RigidBodySystem, GuiSystem,
    CharacterSystem, CameraSystem, DialogSystem, ExaminableSystem, ThreadSystem,
    RenderSystem
*/

/*!
 * \qmlmethod Area* Engine::getCurArea()
 *
 * Returns the current active Area.
 */
/*!
 * \qmlmethod Level* Engine::getCurLevel()
 *
 * Returns the current active Level.
 */
/*!
 * \qmlmethod void Engine::setProcessInput(bool val)
 *
 * Tells the engine to process input normally if \a val is true, otherwise the engine will no longer process input events,
 * giving full control to Lua.
 */
/*!
 * \qmlmethod PartySystem* Engine::getPartySys()
 *
 * Returns the PartySystem subsystem.
 */
/*!
 * \qmlmethod InputSystem* Engine::getInputSys()
 *
 * Returns the InputSystem subsystem.
 */
/*!
 * \qmlmethod TimerSystem* Engine::getTimerSys()
 *
 * Returns the TimerSystem subsystem.
 */
/*!
 * \qmlmethod AudioSystem* Engine::getAudioSys()
 *
 * Returns the AudioSystem subsystem.
 */
/*!
 * \qmlmethod LightSystem* Engine::getLightSys()
 *
 * Returns the LightSystem subsystem.
 */
/*!
 * \qmlmethod LevelSystem* Engine::getLevelSys()
 *
 * Returns the LevelSystem subsystem.
 */
/*!
 * \qmlmethod CombatSystem* Engine::getCombatSys()
 *
 * Returns the CombatSystem subsystem.
 */
/*!
 * \qmlmethod ParticleSystem* Engine::getPartSys()
 *
 * Returns the ParticleSystem subsystem.
 */
/*!
 * \qmlmethod DaySystem* Engine::getDaySys()
 *
 * Returns the DaySystem subsystem.
 */
/*!
 * \qmlmethod CollisionSystem* Engine::getCollisionSys()
 *
 * Returns the CollisionSystem subsystem.
 */
/*!
 * \qmlmethod RigidBodySystem* Engine::getRigidBodySys()
 *
 * Returns the RigidBodySystem subsystem.
 */
/*!
 * \qmlmethod GuiSystem* Engine::getGuiSys()
 *
 * Returns the GuiSystem subsystem.
 */
/*!
 * \qmlmethod ExaminableSystem* Engine::getExamSys()
 *
 * Returns the ExaminableSystem subsystem.
 */
/*!
 * \qmlmethod CameraSystem* Engine::getCameraSys()
 *
 * Returns the CameraSystem subsystem.
 */
/*!
 * \qmlmethod CharacterSystem* Engine::getCharSys()
 *
 * Returns the CharacterSystem subsystem.
 */
/*!
 * \qmlmethod DialogSystem* Engine::getDialogSys()
 *
 * Returns the DialogSystem subsystem.
 */
/*!
 * \qmlmethod EventSystem* Engine::getEventSys()
 *
 * Returns the EventSystem subsystem.
 */
/*!
 * \qmlmethod RenderSystem* Engine::getRenderSys()
 *
 * Returns the RenderSystem subsystem.
 */
/*!
 * \qmlmethod ThreadSystem* Engine::getThreadSys()
 *
 * Returns the ThreadSystem subsystem.
 */





/*!
    \qmltype gyro
    \since 1.6.0
    \inmodule Engine
    \brief Static class for libGyro constants and enumerations.
*/
/*!
 * \qmlmethod GYWav* gyro::createWav(string path)
 * This function is static.
 * Loads a wave file from the given asset-relative \a{path} and returns a new \l{GYWav}
 * object containing its audio buffer or returns a reference to an existing \l{GYWav}
 * object if it has already been loaded.
 *
 * \e{NOTE: Any WAV created in this manner must also be deleted with destroyWav()!}
 * \sa destroyWav(), GYWav
 */
/*!
 * \qmlmethod void gyro::destroyWav(GYWav* wav)
 * This function is static.
 * Destroys the given \a{wav} object and frees its audio buffer if \a{wav} was the last
 * object still referencing the buffer or simply decrements the internal reference count
 * for the buffer if its still in use by other objects.
 * \sa createWav(), GYWav
 */

/*!
\qmlproperty enumeration gyro::TEX_ORIENT_TYPE

Represents the different possible orientations of a 2D texture.

The orientations can be of:
\list
\li gyro.TEX_ORIENT_NORMAL              - No special orientation
\li gyro.TEX_ORIENT_FLIP_HORIZ          - Textured flipped over the Y axis
\li gyro.TEX_ORIENT_FLIP_VERT           - Textured flipped over the X axis
\li gyro.TEX_ORIENT_ROT_90              - Texture rotated 90 degrees clockwise
\li gyro.TEX_ORIENT_ROT_180             - Texture rotated 180 degrees clockwise
\li gyro.TEX_ORIENT_ROT_270             - Textured rotated 270 degrees clockwise
\li gyro.TEX_ORIENT_FLIP_HORIZ_ROT_90   - Texture flipped over the Y axis then rotated 90 degrees clockwise
\li gyro.TEX_ORIENT_FLIP_HORIZ_ROT_270  - Texture flipped over the Y axis then rotated 270 degrees clockwise
\endlist
*/

/*!
\qmlproperty enumeration gyro::RECT_PLANE_TYPE

Represents the different planes a 2D texture can be applied to. Imagine these as the faces of a cube.

The different possible plane types are:
\list
\li gyro.RECT_PLANE_BOTTOM              - Regular, flat texture
\li gyro.RECT_PLANE_FRONT               - Texture standing up, facing front
\li gyro.RECT_PLANE_LEFT                - Texture standing up, facing left
\li gyro.RECT_PLANE_RIGHT               - Texture standing up, facing right
\li gyro.RECT_PLANE_FRONT_SLOPE_45      - Texture as a 45 degree ramp, facing front
\li gyro.RECT_PLANE_LEFT_SLOPE_45       - Texture as a 45 degree ramp, facing left
\li gyro.RECT_PLANE_RIGHT_SLOPE_45      - Texture as a 45 degree ramp, facing right
\endlist
*/

/*!
\qmlproperty enumeration gyro::COLLIDER_TYPE

Represents the different types of 2D colliders supported by the physics engine.

The different collider primitives are:
\list
\li gyro.COLLIDER_NONE  - No collider
\li gyro.COLLIDER_AABB  - Axis-aligned bounding box (only for rectangles that never rotate)
\li gyro.COLLIDER_OBB   - Oriented bounding box (for rectangles that can rotate)
\li gyro.COLLIDER_RTRI  - Right-triangle
\li gyro.COLLIDER_CIR   - Circle
\endlist
*/







/*!
    \qmltype _G
    \since 1.6.1
    \inmodule Engine
    \brief Global Lua Environment Table

    _G is the global environment table for all scripts, behaviors, modules and
    executable Lua code within the Engine (including ESTk's Lua terminal/console).
    The pairs within this table represent global variable names and their values,
    all Lua contexts have at least read-only access to this table, with or without
    explicitly referencing the variable through _G (you can just leave it off).
    Scripts do not implicitly inherit write-access to this table, so writing to a
    global variable must be done explicitly through the _G.* accessor.

    _G houses built-in Lua functions, and some custom overrides supplied by the engine,
    as well as some utility functions, and the actual userdata metatables for \e every
    C++ object that has been proxied/wrapped to Lua.


    \sa Engine, Toolkit
*/

/*!
 * \qmlmethod void _G::print(string msg, ...)
 *  Custom variadic print routine form the Engine, overriding the built-in print function.
 *  It works the exact same way (any number of any argument), but it does a much
 *  better job of converting data types to strings for displaying them, and is
 *  also Lua proxy/userdata type-aware. The default severity level for anything
 *  printed with this function is "verbose," meaning it's simply for debugging and logging
 *  purposes, and these messages do not represent errors or warnings. In the Toolkit's terminal
 *  window, output from this function is displayed as a light blue/teal color
 *
 *  \e {NOTE: Due to the nature of Lua string concatenation, it's advisable to
 *      pass multiple values as a comma separated (',') list (there will automatically
 *      be spaces appended between them) rather than concatenating everything into a single
 *      string by conjoining with the '..' operator. This will make a lot of tiny temporary
 *      allocations on the heap for each substring and is slower and wasteful. Passing a list
 *      of entries allows the Engine to do the heavy lifting. }
 *
 * \e {NOTE: For warning and error severities, see the other variations of this function.}
 *
 * \sa warnprint, errprint, alertprint, rPrint, dbgpush, dbgpop
 */
/*!
 * \qmlmethod void _G::rPrint(LuaTable table, int maxDepth, string stringName)
 *  Custom "Recursive Print" routine, especially useful for printing Lua table or userdata values,
 *  allowing you to inspect deeply embedded tables and objects. The given \a{table} is traversed,
 *  recursively, including all of its nested subtables, until the \a{maxDepth} limit has been reached.
 *  \a{stringName} provides a display name for the top-level table.
 *
 * \e {NOTE: The default \a{maxDepth} value is 100, and the default \a{stringName} is just
 *  an empty string, so these two arguments are optional}
 *
 * \sa print, serialize
 */
/*!
 * \qmlmethod void _G::warnprint(string msg, ...)
 * Custom logging method, similar to \l{print}, only with the Toolkit logging severity set to warning,
 * meaning the message denotes a non-fatal issue or something that may potentially be something the
 * user may want to look into.
 *
 * Warnings are displayed in dark yellow within the ESTk terminal/log.
 * \sa print, errprint, alertprint
 */
/*!
 * \qmlmethod void _G::errprint(string msg, ...)
 * Custom logging method, similar to \l{print} and \l{warnprint}, only with the Toolkit logging severity set to error,
 * meaning the message contains some sort of important, potentially fatal error message, and the user should fix it.
 *
 * Errors are displayed in dark red within the ESTk terminal/log.
 * \sa print, warnprint, alertprint
 */
/*!
 * \qmlmethod void _G::alertprint(string msg, ...)
 * Custom logging method similar to \l{print}, \l{warnprint}, and \l{errprint}, except the output is displayed as a modal
 * alert box widget with an 'Ok' button.
 *
 * \e { NOTE: This method requires ESTk and depends on the \l{Toolkit} class, meaning you should only be using the  method
 *      in Toolkit-specific triggers or control flow. }
 * \sa print, warnprint, errprint
 */
/*!
 * \qmlmethod void _G::printAdvLog(string msg, ...)
 * Custom logging method similar to \l{print}, only the output is added to the \l{AdventureLog} instead of being logged.
 *
 * \e {NOTE: \a{msg} may contain special \l {Font} mark-up for highlighting keywords and/or more customizing the text's appearance}
 *
 * \sa print, warnprint, errprint, alertprint
 */
/*!
 * \qmlmethod void _G::dbgpush()
 * Utility function used to increase the depth of the debug log stack, meaning all following print/warnprint/errprint
 * log messages are considered children of the previous message. This is recommended at the top of a function with
 * complex, multi-layer/aggregate debug output, so that it can be grouped together, formatted and tabbed prettily,
 * and can be collapsed/expanded within the debug output window in ESTk.
 *
 * \sa dbgpop, print
 */
/*!
 * \qmlmethod void _G::dbgpop(int count=1)
 * Utility function used to decrease the depth of the debug log stack (by \a{count}) meaning the following log messages are no longer
 * children of the previous parent, and are instead children to the parent that was active before the last \a{count} pushes. A corresponding
 * dbgpop() is recommended to go at the end of a complex/aggregate function with multiple layers of logging where dbgpush() was
 * previously used.
 *
 * \e {NOTE: \a{count} defaults to 1, which means it will do the exact opposite of the dbgpush function when it's not passed an arguments.
 *      Under certain circumstances you may have nested multilayer logic requiring multiple calls to dbgpush() before calling a dbgpop(),
 *      The two dbgpop() calls can be combined into a single call, with the \a{count} argument set to 2... This can make the code cleaner
 *      (and faster) by allowing you to combine multiple back-to-back dbgpop calls. }
 *
 * \e {NOTE: It is VERY important that for every call to dbgpush at the beginning of a block of logic, there is a corresponding dbgpop(),
 *      otherwise the stack can become incorrect, as all later debug messages will no longer have the correct parent, screwing up the entire
 *      grouping. Also note that poping more to the stack than you push is invalid, and may cause the program to crash. }
 * \sa dbgpush, print
 */
/*!
 * \qmlmethod string _G::serialize(LuaTable ref)
 * Utility function provided by the Engine for converting any arbirary Lua table, userdata, or value (given by \a{ref}) into a single executable string of Lua code
 * in the form of a simple table/object declaration, which may then be written to a file and directly executed later to fully recover the state of the data.
 * The format of this serialized string data is being tenderly named "LSON" as it is the exact Lua equivalent of the "JSON" format... (copyright Falco Girgis)
 *
 * The function is very complex and is the basis for all LSON asset files within ESTk. There are some general considerations to keep in mind when using it:
 * \list
 * \li \b {Userdata} - The values saved for C/++ userdata types are the PROPERTIES that have been wrapped to Lua. A userdata type without properties cannot be
 *      serialized properly, and will just have a placeholder "userdata" string.
 * \li \b {Recursive tables} - If table A contains a reference to table B which then contains a reference to table A, this is considered a recursively
 *      referenced Lua table and cannot be totally represented as serial data. The function will store an error message string rather than a reference back
 *      to the parent once recursion has been detected
 * \li \b {Prefabs} - When a table has been instantiated from a prefab value, the serialize function should attempt to honor this association, by instantiating
 *      the object with the prefab as the parent data, rather than creating an entirely new object
 * \li \b {Data Sharing} - ESTk tries to be as optimal as possible with not bloating save files. It does this by NOT duplicating data that is already present with a
 *      parent prefab table. It also attempts to skip writing out read-only values as well as values which have not been changed from their defaults.
 * \endlist
 *
 * \sa rPrint
 */

namespace elysian::lua {
using namespace elysian;
using Engine = elysian::Engine;

void registerEngine(sol::state_view state) {
    if (state["Engine"].valid()) return;
    state.create_named_table("Engine");
}

void registergyro(sol::state_view state) {
    if (state["gyro"].valid()) return;
    state.create_named_table("gyro");
}
}
