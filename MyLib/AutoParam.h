#pragma once
#include <functional>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>

template<typename TVal>
class AutoParam;

inline void updateWidget(bool v, QCheckBox* w);
inline void connectChange(AutoParam<bool>* target, QCheckBox* w);


template<typename TEnum>
const char** enumStrings();

#define ENUM_STRINGS(name, ...) \
template<> inline const char** enumStrings<name>() { \
    static const char* a[] = { __VA_ARGS__, nullptr }; \
    return a; \
}

class ParamAggregate : public QObject
{
    Q_OBJECT;
public:
signals:
    void changed();
};

class AutoParamBase : public QObject
{
    Q_OBJECT;
signals:
    void changed();
};

template<typename TVal>
class AutoParam : public AutoParamBase
{
public:
    AutoParam(TVal value = TVal(), ParamAggregate* agg = nullptr)
        : m_val(value)
    {
        if (agg)
            connect(this, &AutoParamBase::changed, agg, &ParamAggregate::changed);
    }

    template<typename TWidget>
    void mate(TWidget* widget) {
        m_update = [widget](TVal v) { updateWidget(v, widget); };
        connectChange(this, widget);
        m_update(m_val);
    }

    void operator=(TVal v) {
        if (m_val == v)
            return;
        m_val = v;
        m_update(m_val);
        emit changed();
    }

    void valueFromWidget(TVal v) {
        m_val = v;
        emit changed();
    }

    operator TVal() {
        return m_val;
    }


    TVal m_val;
    std::function<void(TVal)> m_update;
};



inline void updateWidget(bool v, QCheckBox* w) {
    w->setChecked(v);
}
inline void connectChange(AutoParam<bool>* target, QCheckBox* w) {
    QObject::connect(w, &QCheckBox::toggled, [target](bool v) {
        target->valueFromWidget(v);
    });
}

inline void updateWidget(float v, QDoubleSpinBox* w) {
    w->setValue(v);
}
inline void connectChange(AutoParam<float>* target, QDoubleSpinBox* w) {
    QObject::connect(w, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [target](double v) {
        target->valueFromWidget(v);
    });
}

inline void updateWidget(int v, QSpinBox* w) {
    w->setValue(v);
}
inline void connectChange(AutoParam<int>* target, QSpinBox* w) {
    QObject::connect(w, QOverload<int>::of(&QSpinBox::valueChanged), [target](int v) {
        target->valueFromWidget(v);
    });
}

template<typename TEnum>
inline void updateWidget(TEnum v, QComboBox* w) {
    w->setCurrentIndex((int)v);
}
template<typename TEnum>
inline void connectChange(AutoParam<TEnum>* target, QComboBox* w) {
    const char** name = enumStrings<TEnum>();
    while (*name != nullptr)
        w->addItem(*name++);
    QObject::connect(w, QOverload<int>::of(&QComboBox::activated), [target](int v) {
        target->valueFromWidget((TEnum)v);
    });
}

