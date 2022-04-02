#pragma once

#include <string>
#include <typeinfo>
#include <memory>
#include <exception>
#include <unordered_map>
#include <gen/Header.hpp>
#include <base/Macros.hpp>
#include <any>
#include <variant>

#ifndef GEODE_DLL
#define GEODE_DLL
#endif

namespace geode {
    class NotificationCenter;
    class Mod;

    template <typename T = std::monostate>
    struct GEODE_DLL NotifInfo {
        std::string selector;
        NotifInfo(std::string sel) : selector(sel) {}
        NotifInfo() {}
    };

    template <typename T = std::monostate>
    struct GEODE_DLL ConstNotifInfo {
        char const* selector;
        constexpr ConstNotifInfo(char const* sel) : selector(sel) {}
        constexpr ConstNotifInfo() {}

        operator NotifInfo<T>() {
            return NotifInfo<T>(selector);
        }
    };

    template <typename T = std::monostate>
    class GEODE_DLL Notification {
    protected:
        NotifInfo<T> m_info;
        T m_object;
        Mod* m_sender;
    public:
        T const& object() const {
        	return m_object;
        }

        inline std::string const& selector() const { return m_info.selector; }
        inline Mod* sender() const { return m_sender; }

        Notification(NotifInfo<T> inf, T obj, Mod* sender) :
            m_info(inf),
            m_object(obj),
            m_sender(sender) {}

        Notification(std::string sel, T obj, Mod* sender) : Notification(NotifInfo<T>(sel), obj, sender) {}

        Notification(std::string sel, Mod* sender) : Notification(sel, T(), sender) {}
        Notification(NotifInfo<T> inf, Mod* sender) : Notification(inf, T(), sender) {}
        // Notification(std::string sel) : Notification(sel, Interface::get()->mod()) {}

        Notification(Notification&& a) : m_info(a.m_info), m_sender(a.m_sender), m_object(std::move(a.m_object)) {}

        friend class NotificationCenter;
    };

    template <typename T = std::monostate>
    struct Observer {
        NotifInfo<T> m_info;
        Mod* m_mod;
        std::function<void(Notification<T> const&)> m_callback;

        template <typename U = std::monostate>
        Observer<U>* into() {return reinterpret_cast<Observer<U>*>(this);}
    };

    class GEODE_DLL NotificationCenter {
    public:
        std::map<Mod*, std::map<std::string, std::vector<Observer<std::monostate>*>>> m_observers;
        static NotificationCenter* shared;
    public:

        NotificationCenter();
        static NotificationCenter* get();

        template <typename T>
        void send(Notification<T> n, Mod* m) {
            for (auto& obs : m_observers[m][n.selector()]) {
                obs->template into<T>()->m_callback(n);
            }
        }

        template <typename T>
        void broadcast(Notification<T> n) {
            for (auto& [k, v] : m_observers) {
                for (auto& obs : v[n.selector()]) {
                    obs->template into<T>()->m_callback(n);
                }
            }
        }

        template <typename T = std::monostate>
        Observer<std::monostate>* registerObserver(Mod* m, NotifInfo<T> info, std::function<void(Notification<T> const&)> cb) {
            Observer<T>* ob = new Observer<T>;
            ob->m_info = info;
            ob->m_callback = cb;
            ob->m_mod = m;

            m_observers[m][info.selector].push_back(ob->into());

            return ob->into();
        }

        template <typename T = std::monostate>
        Observer<std::monostate>* registerObserver(Mod* m, std::string sel, std::function<void(Notification<T> const&)> cb) {
            return registerObserver(m, NotifInfo<T>(sel), cb);
        }

        template <typename T = std::monostate>
        inline Observer<std::monostate>* registerObserver(std::string sel, std::function<void(Notification<T> const&)> cb);

        template <typename T = std::monostate>
        inline Observer<std::monostate>* registerObserver(NotifInfo<T> info, std::function<void(Notification<T> const&)> cb);

        void unregisterObserver(Observer<std::monostate>* ob);
        std::vector<Observer<std::monostate>*> getObservers(std::string selector, Mod* m);
    };
}
#define _$observe3(sel, T, data, ctr) \
    void $_observer##ctr(geode::Notification<T> const&); \
    static auto $_throw##ctr = (([](){ \
        geode::NotificationCenter::get()->registerObserver<T>( \
            sel, $_observer##ctr \
        ); \
    })(), 0); \
    void $_observer##ctr(geode::Notification<T> const& data)

#define _$observe1(sel, ctr) _$observe3(sel, std::monostate, , ctr)

#define $observe(...) GEODE_INVOKE(GEODE_CONCAT(_$observe, GEODE_NUMBER_OF_ARGS(__VA_ARGS__)), __VA_ARGS__, __COUNTER__)

