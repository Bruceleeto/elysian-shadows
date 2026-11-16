#ifndef ELYSIAN_WARP_SYSTEM_PROXY_HPP
#define ELYSIAN_WARP_SYSTEM_PROXY_HPP
#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/system/elysian_warp_system.hpp>
namespace elysian::lua {
void registerWarpSystem(sol::state_view state);
}
#endif
