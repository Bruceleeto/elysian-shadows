#include <AssetIO/proxy/elysian_component_proxy.hpp>
/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>


/*!
    \qmltype Component
    \since 1.6.0
    \inmodule Engine
    \brief Base class for all attachable Entity components.

    Component is the base class from which all other components within the engine are inherited.
    A Component is an object that can be attached to an Entity to give it certain characteristics.
    For example, a solid Entity should have a Collidable component attached to it, and an Entity that
    uses skeletal animations should have a Skeleton component attached to it.

*/

/*!
 * \qmlmethod Entity* Component::getEntity()
 *
 * Returns a reference to the parent Entity the component is currently attached to.
 */




namespace elysian::lua {
using namespace elysian;
using Component = elysian::Component;

void registerComponent(sol::state_view state) {
    if (state["Component"].valid()) return;
    using T = elysian::Component;
    auto type = registerType<T>(state, "Component");
    type["getEntity"] = method<T, Entity*() const>(&T::getEntity);
}
}
