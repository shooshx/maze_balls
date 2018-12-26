#pragma once
#include <functional>
#include <string>
#include <vector>
#include <sstream>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSettings>

template<typename TVal>
class AutoParam;

inline void updateWidget(bool v, QCheckBox* w);
inline void connectChange(AutoParam<bool>* target, QCheckBox* w);

template<typename TVal>
inline std::string serialize(TVal v) {
    std::stringstream ss; ss << v;
    return ss.str();
}
template<typename TVal>
void deserialize(const char* s, TVal* pv) {
    std::stringstream ss(s);
    ss >> *pv;
}


template<typename TEnum>
const char** enumStrings();

#define ENUM_STRINGS(name, ...) \
template<> inline const char** enumStrings<name>() { \
    static const char* a[] = { __VA_ARGS__, nullptr }; \
    return a; \
} \
template<> inline std::string serialize(name v) { \
    return enumStrings<name>()[(int)v]; \
} \
template<> inline void deserialize(const char* s, name* pv) { \
    const char** enames = enumStrings<name>(); int i = 0; \
    while (enames[i] != nullptr && strcmp(enames[i], s) != 0) \
        i++; \
    *pv = (name)i; \
}

class AutoParamBase : public QObject
{
    Q_OBJECT;
public:
    AutoParamBase(const char* name) : m_name(name) {}
    virtual std::string serialize() = 0;
    virtual void deserialize(const char* s) = 0;
    virtual void reset() = 0;

    const char* m_name = nullptr;
    bool m_dirty = true;
signals:
    void changed();
};


class ParamAggregate : public QObject
{
    Q_OBJECT;
public:
    ParamAggregate(const std::string& org, const std::string& app) :m_org(org), m_app(app) {
        connect(this, &ParamAggregate::changed, this, &ParamAggregate::saveToReg);
    }
    void add(AutoParamBase* p) {
        connect(p, &AutoParamBase::changed, this, &ParamAggregate::changed);
        m_params.push_back(p);
    }
    void resetToDefault() {
        for (auto* p : m_params)
            p->reset();
    }
private:
    std::vector<AutoParamBase*> m_params;
    std::string m_org, m_app;
signals:
    void changed();
public slots:
    void saveToReg() {
        QSettings sett(m_org.c_str(), m_app.c_str());
        for(auto* p: m_params) {
            if (!p->m_dirty)
                continue;
            sett.setValue(p->m_name, QString(p->serialize().c_str()));
            p->m_dirty = false;
        }
    }
    void loadFromReg() {
        QSettings sett(m_org.c_str(), m_app.c_str());
        for (auto* p : m_params) {
            QVariant v = sett.value(p->m_name);
            if (!v.isValid())
                continue;
            p->deserialize(v.toString().toLatin1());
            p->m_dirty = false;
        }
    }
};

#define AUTO_PARAM(type, name, default_value) AutoParam<type> name{default_value, #name, this};

template<typename TVal>
class AutoParam : public AutoParamBase
{
public:
    AutoParam(TVal value = TVal(), const char* name = nullptr, ParamAggregate* agg = nullptr)
        : AutoParamBase(name), m_val(value), m_default(value)
    {
        if (agg)
            agg->add(this);    
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
        m_dirty = true;
        m_update(m_val);
        emit changed();
    }

    void valueFromWidget(TVal v) {
        m_val = v;
        m_dirty = true;
        emit changed();
    }

    operator TVal() {
        return m_val;
    }

    virtual std::string serialize() {
        return ::serialize(m_val);
    }
    virtual void deserialize(const char* s) {
        ::deserialize(s, &m_val);
    }
    virtual void reset() {
        *this = m_default; 
    }

    TVal m_val;
    TVal m_default;
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

