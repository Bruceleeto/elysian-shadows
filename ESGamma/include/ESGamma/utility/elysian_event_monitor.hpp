/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_EVENT_MONITOR_HPP
#define ELYSIAN_EVENT_MONITOR_HPP

#include <functional>
#include <ESGamma/crtp/elysian_lua_manager.hpp>

namespace elysian {


class EventMonitor {
private:
    static EventMonitor         _nullMonitor;
    std::function<void(void)>   _cancel   = nullptr;

public:
    virtual					~EventMonitor(void);

    void                    setCancelCallback(std::function<void(void)> func);
    void                    cancel(void);
	virtual void			onStart(void);
	virtual void			onFinish(void);

    static bool				isNull(const EventMonitor*const mon);
    static EventMonitor*	null(void);

};

class EventCallback: public EventMonitor {
private:
	std::function<void(void)> _callback = nullptr;

public:

    void			setCallbackFunction(std::function<void(void)> func);
	virtual void	onFinish(void) override;
};

class LuaEventMonitor: public EventMonitor, public LuaManager {
protected:
		sol::object	_table;
public:
		void					setTable(const sol::object& table);
		sol::object&	getTable(void);
};



class LuaEventCallback: public LuaEventMonitor {
private:
	sol::protected_function		_callback;

public:
					LuaEventCallback(const sol::protected_function& callback);
	void			setCallbackFunction(const sol::protected_function& callback);
	virtual void	onFinish(void) override;
};










// =========== INLINEZ ===========


inline void EventMonitor::setCancelCallback(std::function<void(void)> func) {
    _cancel = std::move(func);
}
inline EventMonitor::~EventMonitor(void) {
    cancel();
}
inline void EventMonitor::cancel(void) {
     if(_cancel) {
         _cancel();
         _cancel = nullptr;
     }
 }

inline bool EventMonitor::isNull(const EventMonitor*const mon) { return (mon == &_nullMonitor); }
inline EventMonitor* EventMonitor::null(void) { return &_nullMonitor; }
inline void EventMonitor::onStart(void) { }
inline void EventMonitor::onFinish(void) { _cancel = nullptr; }


inline void EventCallback::setCallbackFunction(std::function<void(void)> func) {
    _callback = std::move(func);
}

inline void EventCallback::onFinish(void) {
    EventMonitor::onFinish();
    _callback();
}

inline void LuaEventMonitor::setTable(const sol::object& table) { _table = table; }
inline sol::object& LuaEventMonitor::getTable(void) { return _table; }

inline LuaEventCallback::LuaEventCallback(const sol::protected_function &callback):
    _callback(callback) {}
inline void LuaEventCallback::setCallbackFunction(const sol::protected_function &callback) {
    _callback = callback;
}


}

#endif // ELYSIAN_EVENT_MONITOR_HPP
