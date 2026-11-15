/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef TK_LINE_CHART_H
#define TK_LINE_CHART_H

#include <AssetIO/utility/elysian_lua_values.hpp>
#include <ESGamma/utility/elysian_performance_monitor.hpp>
#include <type_traits>
#include <QtCharts>

namespace QESTkUtils {
#if 0
template<typename T>
class RecursiveSeriesAdapter {
    static const char* 	getName(const T& t) const { return t.getName(); }
    static double 		getValue(const T& t) const { t.getValue(); }
    static size_t		getChildCount(const T& t) { const { t.getChildCount(); }
    static const T& 	getChild(const T& t, unsigned idx) { t.getChild(idx); }
};

template<>
class RecursiveSeriesAdapter<sol::object> {
    static const char* 			getName(const sol::object& t) const { return elysian::lua::get(t, "name").as<const char*>(); }
    static double 				getValue(const sol::object& t) const { return elysian::lua::get(t, "value").as<double>(); }
    static size_t				getChildCount(const sol::object& t) { const { return elysian::lua::get(t, "children").getSize(); }
    static const sol::object& 	getChild(const sol::object& t, unsigned idx) { return elysian::lua::get(elysian::lua::get(t, "children"), idx); }
};


template<typename T>
class AbstractElementAdapter {

};

template<typename T>
class AbstractSeries {
    virtual void update(void) = 0;
};

template<typename T,
        typename  TSr,
        typename  TSl,
        typename  TAd = RecursiveSeriesAdapter<T>>
class RecursiveSeries:
    public AbstractRecursiveSeries,
    public TSr,
    public TSl
{
private:
    T* 		_data = nullptr;

    RecursiveSeries(T* data):
        _data(data)
    {
        setName(TAd::getName(data));
        setValue(TAd::getValue(data));

        for(size_t i = 0; i < TAd::getChildCount(data); ++i) {
            append(RecursiveSeries<T, Tsr, TSl, Tad>(TAd::getChild(data, i)));
        }
    }

    virtual void update(void) override {
        setValue(TAd::getValue(data));
        for(auto& slice : slices())
            static_cast<RecursiveSeries<T, Tsr, TSl, Tad>*>(slice)->update();
    }
};

#endif

//         typename std::enable_if_t<std::is_convertible<T, V>::value, void>* = nullptr
template<typename T,
         typename V>
struct FrameUpdatedValueAdapter {
private:
    const T* _ptr = nullptr;
public:
    FrameUpdatedValueAdapter(const T* t):
        _ptr(t){}

    V operator()(void) { return static_cast<V>(*_ptr); }
};

template<typename T>
struct FrameUpdatedValueAdapter<T, std::function<T(void)>> {
private:
    std::function<T(void)> _func;
public:
    FrameUpdatedValueAdapter(T t):
        _func(std::move(t)) {}

    inline T operator()(void) { return _func(); }
};

template<typename T>
struct FrameUpdatedValueAdapter<T, sol::object> {
private:
    sol::object _v;
public:
    FrameUpdatedValueAdapter(sol::object v):
        _v(std::move(v)) {}

    inline T operator()(void) {
        switch(elysian::lua::type(_v)) {
        case LUA_TFUNCTION:
            return elysian::lua::call(_v).template as<T>();
        default:
            return _v.as<T>();
        }
    }
};

template<>
struct FrameUpdatedValueAdapter<qreal, elysian::PerformanceMonitor> {
private:
    elysian::PerformanceMonitor*    _perfMon = nullptr;
public:
    FrameUpdatedValueAdapter(elysian::PerformanceMonitor* perfMon):
        _perfMon(perfMon) {}

    inline qreal operator()(void) {
        return _perfMon->getTicks();
    }
};

struct AbstractFrameUpdatedValueSeries: public QLineSeries  {
    virtual         ~AbstractFrameUpdatedValueSeries(void) = default;
    virtual void    update(uint64_t frame) = 0;
};


template<typename T,
         typename V,
         typename A=FrameUpdatedValueAdapter<V, T>>
class FrameUpdatedValueSeries: public AbstractFrameUpdatedValueSeries {
private:
    A    _adapter;
public:
    FrameUpdatedValueSeries(A adapter, QString name=QString()):
        _adapter(std::move(adapter))
    {
        setName(name);
    }

    virtual void update(uint64_t frame) override {
        auto val = _adapter();
        append(frame, val);
    }
};



#if 0
class PerformanceMonitorSeries:
    public QAbstractSeries:
    public QAbstractSlice
{

private:
    PerformanceMonitor* monitor = nullptr;



public:
    PerformanceMonitorSeries(PerformanceMonitor* monitor);
    PerformanceMonitor*	getPerformanceMonitor(void);
    void update(bool recursive=false);

}

inline PerformanceMonitorSeries::PerformanceMonitorSeries(PerformanceMonitor* monitor):
    _monitor(monitor)
{
    setName(monitor->getName());
    setValue(monitor->getTicks());
    for(auto* child : monitor->getChildren()) {
        auto* childSlice = new PerformanceMonitorSeries(child);
        append(childSlice);
    }
}

inline void PerformanceMonitorSeries::update(bool recursive) {

}


class ExplodablePieGraph: public QChart {
    void changeSeries(QAbstractSeries *series);


};
#endif


class LineChartView: public QChartView {
    Q_OBJECT
private:
    uint64_t    _curFrame = 0;

    QVector<AbstractFrameUpdatedValueSeries*>   _series;
    QChart*                                     _chart = nullptr;

    void        _applyTheme(void);

protected:
    virtual void changeEvent(QEvent* event) override;

public Q_SLOTS:
    void        onEngineUpdated(void);

public:
                LineChartView(void);
    virtual     ~LineChartView(void) = default;
    unsigned    addSeries(AbstractFrameUpdatedValueSeries* series);
    void        advanceFrame(void);
};

}


#endif // TK_LINE_CHART_H
