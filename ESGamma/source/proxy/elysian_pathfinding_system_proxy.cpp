/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESGamma/proxy/elysian_pathfinding_system_proxy.hpp>


namespace elysian::lua {
using namespace elysian;
using PathfindingSystemNode = elysian::PathfindingSystem::Node;
using PathfindingSystem = elysian::PathfindingSystem;

void registerPathfindingSystemNode(sol::state_view state) {
    if (state["PathfindingSystemNode"].valid()) return;
    using T = elysian::PathfindingSystem::Node;
    auto type = registerType<T>(state, "PathfindingSystemNode");
    type["new"] = sol::factories(factory<T>());
    type["getPredecessor"] = method<T, PathfindingSystemNode*() const>(&T::getPredecessor);
    type["getDistance"] = method<T, int() const>(&T::getDistance);
}

void registerPathfindingSystem(sol::state_view state) {
    if (state["PathfindingSystem"].valid()) return;
    using T = elysian::PathfindingSystem;
    auto type = registerType<T>(state, "PathfindingSystem");
    type["findAllPathsTo"] = method<T, void(const unsigned short, const unsigned short, const int)>(&T::findAllPathsTo);
    type["getTileNode"] = method<T, const PathfindingSystemNode*(const unsigned short, const unsigned short) const>(&T::getTileNode);
}
}
