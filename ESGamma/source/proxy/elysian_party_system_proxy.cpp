/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>
#include <ESGamma/proxy/elysian_party_system_proxy.hpp>

/*!
    \qmltype PartySystem
    \since 1.6.0
    \inmodule Engine
    \brief Engine subsystem for managing the party and its members.

    The PartySystem is reponsible for managing a list of \l{Entity} objects representing
    the current party members. In addition to providing pathing logic for secondary party
    members to follow the party leader, the PartySystem is what is responsible for adding
    and removing party members to and from the current \l{Area} when it is swapped out for
    a new one. PartySystem also ensures that any items or entities held by party members
    during a warp results in them persisting into the next \l{Area}.

    \section2 Retrieving the PartySystem
    The PartySystem object can be retrieved through the static \l{Engine} class:
    \code
    partySys = Engine.getPartySys()
    \endcode

    \sa Engine
*/


/*!
 * \qmlmethod void PartySystem::addCharacterToParty(Entity* entity)
 *  Adds the \l{Entity} given by \a{entity} to the \l{PartySystem} and assumes ownership over it.
 *  Note that \a{entity} must have a valid \l{Character} component and its necessary prerequisites.
 * \sa removeCharacterFromParty(), getCharacterPartyMember()
 */
/*!
 * \qmlmethod void PartySystem::removeCharacterFromParty(Entity* entity)
 *  Removes the \l{Entity} given by \a{entity} from the \l{PartySystem} and returns ownership of it
 *  back to Lua. Returns true if the removal was successful or false otherwise (probably wasn't actually
 *  even within the party to begin with).
 * \sa addCharacterToParty(), getCharacterPartyMember()
 */
/*!
 * \qmlmethod void PartySystem::setPartyLeader(Entity* entity)
 *  Sets the \l{Entity} given by \a{entity} as the leader of the Party, shifting any other party members
 * back by a slot.
 *
 * \e{NOTE: \a{entity} needs to have already been added to the party!}
 * \sa getPartyLeader()
 */
/*!
 * \qmlmethod void PartySystem::setFollowLogicEnabled(bool enabled)
 *  If true, party members who are not the leader automatically follow the leader. If false, members of the party do not
 *  automatically move or follow the leader.
 */
/*!
 * \qmlmethod Entity* PartySystem::getPartyLeader()
 * Returns a reference to the \l{Entity} at the first slot of the party or nil if the party is empty.
 * \sa setPartyLeader()
 */
/*!
 * \qmlmethod int PartySystem::getPartySlot(Entity* entity)
 * Returns the slot index for the party member given by \a{entity} or -1 if \a{entity} is not a valid party member.
 * \e{NOTE: The slot is determined by doing a O(n) linear search for the given \l{Entity}}
 */
/*!
 * \qmlmethod Entity* PartySystem::getCharacterPartyMember(unsigned slot)
 * Returns a reference to the \l{Entity} party member at the given \a{slot} index or nil if there isn't one
 * at the given slot.
 * \sa addCharacterToParty(), removeCharacterFromParty()
 */
/*!
 * \qmlmethod unsigned PartySystem::getPartySize()
 * Returns the number of party members currently within the party.
 */
/*!
 * \qmlmethod int PartySystem::getPlayerIndexFromSlot(int slot)
 * If \a{slot} is valid and is controlled by a human player, returns the player's index.
 * If \a{slot} is valid and is not controlled by a human player, returns \a{UnassignedIndex}.
 * If \a{slot} is not a valid party slot, returns \a{InvalidIndex}.
 */

/*!
 * \qmlmethod int PartySystem::getPlayerIndexFromPartyMember(Entity* entity)
 * If \a{entity} is a valid party member and is controlled by a human player, returns the player's index.
 * If \a{entity} is valid party member and is not controlled by a human player, returns \a{UnassignedIndex}.
 * If \a{entity} a party member, returns \a{InvalidIndex}.
 */

namespace elysian::lua {
using namespace elysian;
using PartySystem = elysian::PartySystem;

void registerPartySystem(sol::state_view state) {
    if (state["PartySystem"].valid()) return;
    using T = elysian::PartySystem;
    auto type = registerType<T>(state, "PartySystem");
    type["addCharacterToParty"] = [](T& self, sol::object arg0) { return std::invoke(method<T, void(Entity*)>(&T::addCharacterToParty), self, take<Entity>(arg0)); };
    type["removeCharacterFromParty"] = [](T& self, const Entity* arg0) { auto result = std::invoke(method<T, bool(const Entity*)>(&T::removeCharacterFromParty), self, arg0); return std::make_tuple(result, arg0); };
    type["getPartyLeader"] = method<T, Entity*() const>(&T::getPartyLeader);
    type["getCharacterPartyMember"] = method<T, Entity*(unsigned) const>(&T::getCharacterPartyMember);
    type["getPartySize"] = method<T, unsigned() const>(&T::getPartySize);
    type["setPartyLeader"] = method<T, void(Entity*const)>(&T::setPartyLeader);
    type["getPartySlot"] = method<T, int(const Entity*) const>(&T::getPartySlot);
    type["addPlayable"] = [](T& self, sol::object arg0) { return std::invoke(method<T, void(Entity*)>(&T::addPlayable), self, take<Entity>(arg0)); };
    type["getPlayable"] = method<T, Entity*(const char*) const>(&T::getPlayable);
    type["setFollowLogicEnabled"] = method<T, void(bool)>(&T::setFollowLogicEnabled);
    type["getPlayerIndexFromSlot"] = method<T, int(int) const>(&T::getPlayerIndexFromSlot);
    type["getPlayerIndexFromPartyMember"] = method<T, int(const Entity*) const>(&T::getPlayerIndexFromPartyMember);
}
}
