#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <cassert>
#include <iostream>
#include <fstream>

struct Prefix { virtual ~Prefix() = default; int padding = 17; };
struct Base {
    int value = 1;
    int get() const { return value; }
    void set(int v) { value = v; }
};
struct Object : Prefix, Base {
    static inline int destroyed = 0;
    std::vector<int> items{1, 2};
    std::size_t count() const { return items.size(); }
    int item(std::size_t i) const { return items.at(i); }
    void setItem(std::size_t i, int v) { if (items.size() <= i) items.resize(i + 1); items[i] = v; }
    ~Object() { ++destroyed; }
};

struct Shared {
    int payload = 37;
    static inline int destroyed = 0;
    ~Shared() { ++destroyed; }
};

struct SharedDerived : Prefix, Shared {};

struct Position {
    int x = 0;
    explicit Position(int value = 0) : x(value) {}
    int get() const { return x; }
    void set(int value) { x = value; }
};

struct CollisionMask {
    uint32_t value = 0;
    bool getBit(unsigned index) const { return (value & (uint32_t{1} << index)) != 0; }
    void setBit(unsigned index, bool enabled) {
        const auto bit = uint32_t{1} << index;
        if(enabled) value |= bit;
        else value &= ~bit;
    }
    unsigned getBitCount() const { return 32; }
    uint32_t getValue() const { return value; }
};

struct Node {
    Node* child = nullptr;
    int value = 0;
};

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string, sol::lib::math);
    elysian::lua::registerView(lua);
    elysian::lua::registerType<Base>(lua, "Base");
    auto type = elysian::lua::registerType<Object, Base>(lua, "Object");
    type["new"] = sol::factories([] { return elysian::lua::own<Object>(); });
    elysian::lua::property(type, "value", elysian::lua::method<Object, int() const>(&Object::get),
        elysian::lua::method<Object, void(int)>(&Object::set));
    elysian::lua::property(type, "readonly", elysian::lua::method<Object, int() const>(&Object::get));
    bool serializing = false;
    elysian::lua::viewProperty(type, elysian::lua::group("nested", {
        elysian::lua::field<Object>("value", &Object::get, &Object::set),
        elysian::lua::field<Object>("readonly", &Object::get)
    }, [&] { return serializing; }));
    elysian::lua::viewProperty(type, elysian::lua::array<Object>("items", &Object::item, &Object::setItem, &Object::count));
    elysian::lua::tableConstructor(type);
    elysian::lua::finish<Object>(type, "Object", [&] { return serializing; });
    Object* taken = nullptr;
    lua.set_function("take", [&](sol::object obj) { taken = elysian::lua::take<Object>(obj); });
    auto run = [&](const char* code) {
        std::string lines(code);
        for (std::size_t p = 0; (p = lines.find(';', p)) != std::string::npos; p += 2) lines.insert(p + 1, "\n");
        auto result = lua.safe_script(lines, sol::script_pass_on_error);
        if (!result.valid()) { std::cerr << sol::error(result).what() << '\n'; std::abort(); }
    };
    run("assert(Base.new==nil)");
    run("o = Object.new{value=42}; assert(o.value == 42); assert(o.__typename == 'Object'); "
        "o:fromTable{value = 7}; assert(o.value == 7); "
        "local n = 0; for k,v in pairs(o) do n=n+1 end; assert(n==4); "
        "local view=o.nested; view.value=8; assert(o.value==8); "
        "assert(tostring(view)=='userdata[Object.nested]: virtual proxy'); "
        "assert(tostring(o.items)=='userdata[Object.items]: array proxy'); "
        "o.items={3,4,5}; assert(#o.items==3 and o.items[3]==5); "
        "local sum=0; for i,v in pairs(o.items) do sum=sum+v end; assert(sum==12); "
        "o.value=7; local copy=o:new{value=19}; assert(copy.value==19 and o.value==7)");
    serializing = true;
    run("local n = 0; for k,v in pairs(o) do n=n+1; assert(k~='readonly') end; assert(n==3); "
        "local n = 0; for k,v in pairs(o['nested']) do n=n+1; assert(k=='value') end; assert(n==1); "
        "take(o); o.value=11; assert(o.value==11); o=nil; collectgarbage('collect'); collectgarbage('collect'); collectgarbage('collect')");
    assert(taken && taken->value == 11 && Object::destroyed == 1);
    delete taken;
    run("local x=Object.new(); x=nil; collectgarbage('collect'); collectgarbage('collect'); collectgarbage('collect')");
    assert(Object::destroyed == 3);
    run("local x=Object:new{value=6}; local y=Object:new(); local z=x:new(); "
        "assert(x.value==6 and y.value==1 and z.value==6); "
        "local ok=pcall(function() x.readonly=1 end); assert(not ok); "
        "x.value=3.5; assert(x.value==4); x.value=-3.5; assert(x.value==-4)");

    auto position = elysian::lua::registerType<Position>(lua, "Position");
    position["new"] = sol::factories(elysian::lua::factory<Position>(), elysian::lua::factory<Position, int>());
    elysian::lua::property(position, "x", &Position::get, &Position::set);
    elysian::lua::finish<Position>(position, "Position", [] { return false; });
    elysian::lua::tableConstructor(position);
    run("assert(Position.new(8).x==8); assert(Position:new(9).x==9); "
        "assert(Position.new{x=10}.x==10); assert(Position:new{x=11}.x==11)");

    auto shared = elysian::lua::registerType<Shared>(lua, "Shared");
    shared["new"] = sol::factories(elysian::lua::sharedFactory<Shared>());
    std::shared_ptr<Shared> held;
    lua.set_function("hold", [&](std::shared_ptr<Shared> object) { held = std::move(object); });
    run("hold(Shared.new()); collectgarbage('collect')");
    assert(held && Shared::destroyed == 0);
    held.reset();
    assert(Shared::destroyed == 1);
    run("hold(nil); local ok=pcall(function() hold(Object.new()) end); assert(not ok)");
    assert(!held);

    lua.set_function("nullable", [](sol::optional<sol::table> value) { return value ? value->size() : 0; });
    run("assert(nullable(nil)==0); assert(nullable({1,2})==2)");
    lua.set_function("nullObject", []() -> Object* { return nullptr; });
    run("assert(nullObject()==nil)");

    // A retained view alone keeps its parent alive through collection.
    run("view = Object.new{value=23}['nested']; collectgarbage('collect'); assert(view.value==23); "
        "view.value=24; assert(view.value==24); view=nil; collectgarbage('collect'); collectgarbage('collect')");
    lua.set_function("again", [](Object* object) { return object; });
    lua.set_function("asBase", [](Object* object) -> Base* { return object; });
    run("local object=Object.new(); local parentCache={[object]='parent'}; "
        "assert(rawequal(object, again(object))); assert(parentCache[again(object)]=='parent'); "
        "assert(rawequal(object, asBase(object))); assert(parentCache[asBase(object)]=='parent'); "
        "weak=setmetatable({object}, {__mode='v'}); object=nil; parentCache=nil; "
        "collectgarbage('collect'); collectgarbage('collect'); assert(weak[1]==nil)");
    lua.set_function("againShared", [](std::shared_ptr<Shared> object) { return object; });
    lua.set_function("rawShared", [](std::shared_ptr<Shared> object) { return object.get(); });
    run("local object=Shared.new(); local parentCache={[object]=true}; "
        "assert(rawequal(object,againShared(object))); assert(rawequal(object,rawShared(object))); "
        "assert(parentCache[againShared(object)] and parentCache[rawShared(object)])");
    Object baseFirst;
    lua["baseFirst"] = static_cast<Base*>(&baseFirst);
    lua["derivedLater"] = &baseFirst;
    run("assert(rawequal(baseFirst,derivedLater)); baseFirst.value=71; assert(derivedLater.value==71)");
    lua["baseFirst"] = sol::nil;
    lua["derivedLater"] = sol::nil;
    auto sharedDerivedType = elysian::lua::registerType<SharedDerived, Shared>(lua, "SharedDerived");
    sharedDerivedType["new"] = sol::factories(elysian::lua::sharedFactory<SharedDerived>());
    run("local derived=SharedDerived.new(); hold(derived); assert(rawequal(derived,rawShared(derived)))");
    assert(held);
    held.reset();
    auto sharedBaseFirst = std::make_shared<SharedDerived>();
    lua["sharedBaseFirst"] = std::static_pointer_cast<Shared>(sharedBaseFirst);
    lua["sharedDerivedLater"] = sharedBaseFirst;
    lua["sharedBaseAgain"] = std::static_pointer_cast<Shared>(sharedBaseFirst);
    run("assert(rawequal(sharedBaseFirst,sharedDerivedLater)); assert(rawequal(sharedBaseAgain,sharedDerivedLater))");
    assert(lua["sharedBaseAgain"].get<std::shared_ptr<SharedDerived>>() == sharedBaseFirst);
    assert(lua["sharedBaseFirst"].get<std::shared_ptr<Shared>>().get() == static_cast<Shared*>(sharedBaseFirst.get()));
    lua["sharedBaseFirst"] = sol::nil;
    lua["sharedDerivedLater"] = sol::nil;
    lua["sharedBaseAgain"] = sol::nil;
    auto rawFirst = std::make_shared<Shared>();
    lua["rawFirst"] = rawFirst.get();
    lua["sharedLater"] = rawFirst;
    run("assert(rawequal(rawFirst,sharedLater)); hold(rawFirst)");
    assert(held == rawFirst);
    rawFirst.reset();
    held.reset();
    run("assert(rawequal(rawFirst,againShared(sharedLater))); rawFirst=nil; sharedLater=nil; collectgarbage('collect'); collectgarbage('collect')");

    auto node = elysian::lua::registerType<Node>(lua, "Node");
    node["new"] = sol::factories(elysian::lua::factory<Node>());
    elysian::lua::property(node, "child", [](Node& n) { return n.child; }, [](Node& n, Node* child) { n.child = child; });
    elysian::lua::property(node, "value", [](Node& n) { return n.value; }, [](Node& n, int value) { n.value = value; });
    elysian::lua::finish<Node>(node, "Node", [] { return true; });
    elysian::lua::tableConstructor(node);
    if (std::ifstream("assets/scripts/utility/table_manip.lua") && std::ifstream("assets/scripts/utility/serialize.lua")) {
        for (const char* path : {"assets/scripts/utility/table_manip.lua", "assets/scripts/utility/serialize.lua"}) {
            auto loaded = lua.safe_script_file(path, sol::script_pass_on_error);
            if (!loaded.valid()) { std::cerr << sol::error(loaded).what() << '\n'; std::abort(); }
        }
        serializing = false;
        run("local o=Object.new{value=13}; local copy=deepcopy(o); "
            "copy.value=15; assert(copy.value==15 and o.value==13); "
            "local nested=deepcopy(o.nested); local items=deepcopy(o.items); "
            "assert(type(nested)=='table' and type(items)=='table'); "
            "nested.value=99; items[1]=99; assert(o.value==13 and o.items[1]==1)");
        run("local n=Node.new{value=13}; local text=serialize(n,nil,true); "
            "local copy=assert(load(text))(); assert(copy.value==13); "
            "n.child=n; text=serialize(n,nil,true); assert(text:find('RECURSIVE TABLES NOT SUPPORTED',1,true)); "
            "local parent={value=9}; setparenttable(n,parent); assert(rawequal(getparenttable(n.child),parent))");
    } else {
        std::cout << "Asset serialization checks skipped: local assets are absent\n";
    }
    assert(lua_gettop(lua) == 0);
    if (std::ifstream("assets/scripts/component/bitMask32.lua")) {
        using Mask = CollisionMask;
        auto mask = elysian::lua::registerType<Mask>(lua, "BitMask32");
        mask["new"] = sol::factories(elysian::lua::factory<Mask>());
        elysian::lua::property(mask, "value", &Mask::getValue);
        elysian::lua::viewProperty(mask, elysian::lua::array<Mask>("bitFields",
            &Mask::getBit, &Mask::setBit, &Mask::getBitCount));
        elysian::lua::finish<Mask>(mask, "BitMask32", [] { return false; });
        elysian::lua::tableConstructor(mask);
        auto loaded = lua.safe_script_file("assets/scripts/component/bitMask32.lua", sol::script_pass_on_error);
        if (!loaded.valid()) { std::cerr << sol::error(loaded).what() << '\n'; std::abort(); }
        run(R"(
            local player = BitMask32.playerMask
            local against = BitMask32.negateMask(player)
            assert(not player.bitFields[1], 'tile bit must not classify players')
            assert(against.bitFields[1], 'party members must collide with walls')
            assert((against.value & player.value) == 0, 'party members must not block one another')
            assert((against.value & BitMask32.enemyMask.value) ~= 0, 'enemies must still collide')
            local door = BitMask32:new { bitFields = { [4] = true } }
            assert((against.value & door.value) ~= 0, 'door warps must still collide')
        )");
    }

    std::cout << "sol2 binding tests passed\n";
}
