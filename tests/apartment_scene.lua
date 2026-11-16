-- Headless regression checks for the actual opening-scene script and area data.
local directory = "assets/level/rewrite/01_apartment/area/apartment/"
local available = io.open(directory .. "apartment_behav.lua", "r")
if not available then return end -- Private assets may not be checked out in CI.
available:close()

local function tableType()
    return { new = function(_, data) return data end }
end
Entity, Vector3, CollidableAttrib, BitMask32, Recti = tableType(), tableType(), tableType(), tableType(), tableType()
Dialog, DialogText = tableType(), tableType()
function prefab() return Entity end
local data = dofile(directory .. "area_table.lua")
local definitions = {}
for _, entry in ipairs(data.activeEntities) do definitions[entry.name] = entry end
assert(definitions["Apartment Eryn"].dispName == "Eryn")
assert(definitions["Apartment Landlord"].dispName == "Landlord")
assert(definitions["Apartment Eryn"].examinable.enabled == false)
assert(definitions["Apartment Landlord"].examinable.enabled == false)
local doorData = definitions["Apartment door"]
assert(doorData.collidable.active == false)
assert(doorData.warp.levelPath == "/assets/level/release/foloi_forest")
assert(doorData.warp.areaPath == "/assets/level/release/foloi_forest/area/loren_town")

Behavior = { ON_ENTER = 1, ON_UPDATE = 2, ON_EXIT = 3 }
Dialog = { new = function() return {
    screens = {},
    addTextScreen = function(self, text)
        local screen = { text = text, nextIndex = #self.screens + 1 }
        function screen:addSourceEntity(entity) self.speaker = entity end
        function screen:setNextIndex(index) self.nextIndex = index end
        self.screens[#self.screens + 1] = screen
        return screen
    end,
} end }
LuaEventCallback = { new = function(callback) return callback end }
dofile(directory .. "apartment_behav.lua")
local triggers = {}
setScriptAttrib({ addSupportedTrigger = function(_, trigger) triggers[trigger] = true end })
assert(triggers[Behavior.ON_ENTER] and triggers[Behavior.ON_UPDATE] and triggers[Behavior.ON_EXIT])

local function fixture(flags, previousInput, previousPause)
    story = flags or {}
    local area = { entities = {}, removed = {} }
    function area:findEntity(name) return self.entities[name] end
    function area:removeEntity(entity)
        assert(self.entities[entity.name] == entity, "Entity removed twice or from the wrong area")
        self.entities[entity.name] = nil
        self.removed[entity.name] = true
    end
    local function entity(name, position)
        local result = { name = name, x = position.x, y = position.y }
        function result:getTransform() return {
            getPosX = function() return self.x end, getPosY = function() return self.y end,
        } end
        area.entities[name] = result
        return result
    end
    local julien = entity("Julien", data.startPosition)
    local members = { Julien = julien }
    for _, name in ipairs({ "Eryn", "Rand", "Alexios" }) do
        members[name] = entity(name, data.startPosition)
    end
    for _, name in ipairs({ "Apartment Eryn", "Apartment Landlord" }) do
        entity(name, definitions[name].transform.pos)
    end
    local door = entity("Apartment door", doorData.transform.pos)
    local collider = { active = false, setActive = function(self, active) self.active = active end }
    function door:getCollidable() return collider end
    local party = { input = previousInput ~= false, roster = {} }
    for name, member in pairs(members) do party.roster[name] = member end
    function party:getPlayable(name) return members[name] end
    function party:getPartySlot(member) return self.roster[member.name] and 0 or -1 end
    function party:removeCharacterFromParty(member)
        assert(self.roster[member.name])
        self.roster[member.name] = nil -- Mirrors the native bug: entity stays in area.
    end
    party.leader = julien
    function party:getPartyLeader() return self.leader end
    function party:isInputEnabled() return self.input end
    function party:setInputEnabled(enabled) self.input = enabled end
    local gui = { pause = previousPause ~= false }
    function gui:isPauseMenuEnabled() return self.pause end
    function gui:setPauseMenuEnabled(enabled) self.pause = enabled end
    local warp = { transitioning = true }
    function warp:isTransitioning() return self.transitioning end
    local timer = { delta = 0.25, getDeltaTime = function(self) return self.delta end }
    local dialogs = { calls = 0, current = nil }
    function dialogs:startDialog(dialog, callback)
        assert(not warp.transitioning, "Dialogue started during the warp")
        assert(not self.current, "Dialogue replaced an active conversation")
        assert(not party.input and not gui.pause)
        assert(#dialog.screens == 3)
        for i, screen in ipairs(dialog.screens) do
            assert(screen.speaker)
            assert(screen.nextIndex == (i == 3 and -1 or i))
            local lines = 0
            for line in screen.text:gmatch("[^\n]+") do
                lines = lines + 1
                assert(#line < 60, "Opening line is not wrapped for the dialogue box")
            end
            assert(lines <= 3)
        end
        self.calls = self.calls + 1
        self.current, self.callback = dialog, callback
    end
    function dialogs:isOpen() return self.current ~= nil end
    function dialogs:closeDialog()
        local callback = self.callback
        if callback then callback() end -- Native callback runs before clearing the dialog.
        self.current, self.callback = nil, nil
    end
    Engine = {
        getCurArea = function() return area end, getPartySys = function() return party end,
        getGuiSys = function() return gui end, getWarpSys = function() return warp end,
        getTimerSys = function() return timer end, getDialogSys = function() return dialogs end,
    }
    return {}, area, party, gui, warp, dialogs, julien, collider
end

local self, area, party, gui, warp, dialogs, julien, door = fixture()
onEnter(self)
for _, name in ipairs({ "Eryn", "Rand", "Alexios" }) do
    assert(not party.roster[name] and area.removed[name])
end
assert(party.roster.Julien and area.entities.Julien)
assert(area.entities["Apartment Eryn"] and not party.input and not gui.pause and not door.active)
for _ = 1, 12 do onUpdate(self) end -- Longer than the wake delay, but still fading.
assert(dialogs.calls == 0 and self.timer == 0)
warp.transitioning = false
onUpdate(self)
onUpdate(self)
assert(dialogs.calls == 0)
onUpdate(self) -- 0.75 seconds after the fade ends.
assert(dialogs.calls == 1 and dialogs.current.screens[1].speaker.name == "Apartment Eryn")
for _ = 1, 4 do onUpdate(self) end
assert(dialogs.calls == 1 and not party.input)
dialogs:closeDialog()
assert(story.apartmentWakeUp and party.input and gui.pause)
assert(area.entities["Apartment Eryn"]) -- Camera cleanup must finish before removal.
onUpdate(self)
assert(area.removed["Apartment Eryn"] and not door.active)
julien.x, julien.y = 820, 3425
onUpdate(self)
assert(dialogs.calls == 2 and dialogs.current.screens[1].speaker.name == "Apartment Landlord")
assert(not party.input and not gui.pause and not door.active)
dialogs:closeDialog()
assert(story.apartmentRent and party.input and gui.pause and door.active)
for _ = 1, 6 do onUpdate(self) end
assert(dialogs.calls == 2)
onExit(self)
onEnter(self) -- Cached area: completed conversations stay completed.
for _ = 1, 6 do onUpdate(self) end
assert(dialogs.calls == 2 and door.active)
onExit(self)

self, area, party, gui, warp, dialogs = fixture()
onEnter(self)
warp.transitioning = false
for _ = 1, 3 do onUpdate(self) end
assert(dialogs:isOpen())
onExit(self) -- Interrupted dialogue cannot set a completion flag.
assert(not dialogs:isOpen() and not story.apartmentWakeUp and party.input and gui.pause)
onUpdate(self)
assert(dialogs.calls == 1)

self, area, party, gui = fixture(nil, false, false)
onEnter(self)
onExit(self)
assert(not party.input and not gui.pause) -- Restore the original settings, not hardcoded true.
self, area, party, gui, warp, dialogs, julien = fixture()
party.leader = nil
onEnter(self)
assert(party.input and not gui.pause) -- Leave initial player assignment enabled.
warp.transitioning = false
onUpdate(self)
assert(dialogs.calls == 0 and self.timer == 0)
party.leader = julien -- Normal PartySystem input now assigns the first player.
for _ = 1, 3 do onUpdate(self) end
assert(dialogs.calls == 1 and not party.input)
onExit(self)
print("Apartment scene: solo roster/area, post-fade delay, three-screen dialogs, cleanup and door gate passed")
