#include <ESGamma/proxy/elysian_warp_system_proxy.hpp>

/*!
    \qmltype WarpSystem
    \inmodule Engine
    \brief Changes levels and areas using the engine's fade transition.
*/
/*!
    \qmlmethod bool WarpSystem::warpTo(string levelPath, string areaPath)
    Starts a fade to the full asset paths given by \a levelPath and \a areaPath.
    The party appears at the destination area's start position, using its camera bounds.
    Returns false if warping is disabled, a transition is already running, paths are
    empty or too long, or the editor cannot find the destination. The transition runs
    on subsequent frames; scripts must not switch areas themselves after calling this.
*/
namespace elysian::lua {
void registerWarpSystem(sol::state_view state) {
    if(state["WarpSystem"].valid()) return;
    using T = elysian::WarpSystem;
    auto type = registerType<T>(state, "WarpSystem");
    type["warpTo"] = method<T, bool(const char*, const char*)>(&T::warpTo);
}
}
