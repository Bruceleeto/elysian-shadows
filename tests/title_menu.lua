-- Exercise the real title behavior with headless stand-ins for engine services.
-- Rendering and C++ ownership still need the user's editor review.
Behavior = { ON_ENTER = 1, ON_EXIT = 2, ON_UPDATE = 3 }
Controller = { BUT_DPAD_UP = 1, BUT_DPAD_DOWN = 2, BUT_X = 3, BUT_CIRCLE = 4,
    BUT_START = 5, ANALOG_LEFT = 0 }
Keyboard = { KEY_RETURN = 1, KEY_D = 2, KEY_DPAD_U = 3, KEY_DPAD_D = 4 }
Widget = { FIXED = 0 }
Color4 = { new = function(value) return value end }
Vector2 = { new = function(x, y) return { x = x, y = y } end }

local function widget()
    return {
        setSizePolicy = function() end,
        setSize = function(self, value) self.size = value end,
        setPosition = function(self, value) self.position = value end,
        setLayout = function(self, value) self.layout = value end,
        setText = function(self, text) self.text = text end,
        setColor = function(self, value) self.color = value end,
    }
end
GroupWidget = { new = widget }
LabelWidget = { new = widget }
EntityWidget = { new = function(entity)
    local result = widget()
    result.entity = entity
    return result
end }
VerticalLayout = { new = function() return {
    children = {}, addWidget = function(self, child) table.insert(self.children, child) end
} end }
SpriteAttrib = { new = function() return { setColor = function() end, loadTexture = function(_, path, w, h, sw, sh)
    assert(path == "/assets/rewrite/background/placeholder/placeholder_background.png")
    assert(w == 500 and h == 500 and sw == w and sh == h)
end } end }
Entity = { new = function() return { addRenderable = function()
    return { setSpriteAttrib = function() end }
end } end }

local party = { input = true, visible = true }
function party:isInputEnabled() return self.input end
function party:isPartyVisible() return self.visible end
function party:setInputEnabled(enabled) self.input = enabled end
function party:setPartyVisible(visible) self.visible = visible end
local previousWidget = {}
local gui = { pause = true, fg = previousWidget, width = 800, height = 600 }
function gui:isPauseMenuEnabled() return self.pause end
function gui:setPauseMenuEnabled(enabled) self.pause = enabled end
function gui:getFgWidget() return self.fg end
function gui:setFgWidget(widget_) self.fg = widget_ end
function gui:getResolution() return self.width, self.height end
local buttons, keys, analog = {}, {}, 0
local controller = {
    isTapped = function(_, button) return buttons[button] end,
    getAnalogStick = function() return { getY = function() return analog end } end,
}
local keyboard = { isTapped = function(_, key) return keys[key] end }
local input = {
    getController = function() return controller end,
    getKeyboard = function() return keyboard end,
}
local warp = { accept = true, calls = 0 }
function warp:warpTo(level, area)
    assert(level == "/assets/level/release/foloi_forest")
    assert(area == level .. "/area/loren_town")
    self.calls = self.calls + 1
    return self.accept
end
Engine = {
    getPartySys = function() return party end, getGuiSys = function() return gui end,
    getInputSys = function() return input end, getWarpSys = function() return warp end,
}
dofile("assets/level/rewrite/00_title/area/title/title_behav.lua")
local triggers = {}
setScriptAttrib({ addSupportedTrigger = function(_, trigger) triggers[trigger] = true end })
assert(triggers[Behavior.ON_ENTER] and triggers[Behavior.ON_EXIT] and triggers[Behavior.ON_UPDATE])
local self = {}
local function frame(button, key, stick)
    buttons, keys, analog = {}, {}, stick or 0
    if button then buttons[button] = true end
    if key then keys[key] = true end
    onUpdate(self)
end
onEnter(self)
assert(not party.input and not party.visible and not gui.pause)
assert(gui.fg == self.root and self.root.entity == self.background)
assert(self.root.size.x == 800 and self.root.size.y == 600)
assert(self.root.position.x == 400 and self.root.position.y == 300)
assert(self.labels[2].text == "> New Game" and self.labels[3].color.r == 110)
local root = self.root
onEnter(self) -- Repeated enter cannot overwrite the original saved controls.
assert(self.root == root)
frame(Controller.BUT_START)
assert(warp.calls == 0 and not gui.pause)
frame(Controller.BUT_DPAD_DOWN)
assert(self.labels[3].text == "> Load")
frame(Controller.BUT_X)
assert(warp.calls == 0 and not party.input and not party.visible)
frame(Controller.BUT_DPAD_DOWN)
frame(Controller.BUT_X)
assert(self.labels[2].text == "Created by the Elysian Shadows Team")
frame(nil, Keyboard.KEY_D)
assert(self.labels[4].text == "> Credits")
frame(nil, Keyboard.KEY_RETURN)
frame(Controller.BUT_CIRCLE)
assert(self.labels[4].text == "> Credits")
frame(nil, Keyboard.KEY_DPAD_D)
assert(self.labels[2].text == "> New Game") -- Wraps from Credits.
frame(nil, nil, 1)
assert(self.labels[3].text == "> Load")
frame(nil, nil, 1)
assert(self.labels[3].text == "> Load") -- Holding stick does not auto-repeat.
frame(nil, nil, -1)
assert(self.labels[2].text == "> New Game")
gui.width, gui.height = 1024, 768
frame()
assert(self.root.size.x == 1024 and self.root.size.y == 768)
assert(self.root.position.x == 512 and self.root.position.y == 384)
warp.accept = false
frame(nil, Keyboard.KEY_RETURN)
assert(warp.calls == 1 and not self.leaving and not party.visible and not gui.pause)
assert(self.labels[5].text:find("Town unavailable", 1, true))
warp.accept = true
frame(nil, Keyboard.KEY_RETURN)
assert(warp.calls == 2 and self.leaving and party.input and party.visible and gui.pause)
assert(gui.fg == root) -- Keep the title under the fade until the area actually changes.
frame(Controller.BUT_X)
assert(warp.calls == 2)
onExit(self)
assert(gui.fg == previousWidget and party.input and party.visible and gui.pause)
assert(not self.root and not self.background)
onExit(self) -- Cleanup is idempotent.
onEnter(self) -- Cached area re-entry builds a fresh menu.
assert(self.root ~= root and not party.visible)
onExit(self) -- Editor area changes restore controls without selecting New Game.
assert(party.input and party.visible and gui.pause and gui.fg == previousWidget)

party.input, party.visible, gui.pause = false, false, false
onEnter(self)
controller, keyboard = nil, nil
frame() -- Disconnected devices are safe.
onExit(self)
assert(not party.input and not party.visible and not gui.pause)
print("Title menu: navigation, disabled Load, Credits, resize, warp and cleanup passed")
