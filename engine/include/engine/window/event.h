#ifndef EVENT_H_
#define EVENT_H_

#include <functional>
#include <algorithm>
#include <typeindex>
#include <unordered_map>
#include <vector>

enum class EventType {
    WINDOW_RESIZE,
    WINDOW_CLOSE,
};

class Event {
public:
    enum EventCategory {
        WINDOW,
        // KEYBOARD ...
    };

    Event(EventCategory category) : m_category(category) {}
    virtual ~Event() {}

    Event(const Event& event) = default;

    inline const EventCategory GetCategory() const { return m_category; }

    inline const virtual EventType GetType() const = 0;
private:
    EventCategory m_category;
};

class EventDispatcher {
    using Type = std::type_index;
    using RawFn = std::function<void(const void*)>;

    struct Slot { std::size_t id; RawFn fn; };

    std::unordered_map<Type, std::vector<Slot>> subs_;
    std::size_t next_id_ = 1;

public:
    struct Handle {
        std::type_index type{typeid(void)};
        std::size_t id{0};
        explicit operator bool() const { return id != 0; }
    };

    template<class E, class F>
    Handle Subscribe(F&& f) {
        auto& vec = subs_[Type(typeid(E))];
        Handle h{ Type(typeid(E)), next_id_++ };
        // Wrap strongly typed callback into type-erased RawFn
        RawFn wrapper = [fn = std::function<void(const E&)>(std::forward<F>(f))](const void* p){
            fn(*static_cast<const E*>(p));
        };
        vec.push_back(Slot{h.id, std::move(wrapper)});
        return h;
    }

    // Unsubscribe with handle
    void Unsubscribe(const Handle& h) {
        auto it = subs_.find(h.type);
        if (it == subs_.end()) return;
        auto& vec = it->second;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                  [&](const Slot& s){ return s.id == h.id; }),
                  vec.end());
    }

    // Publish immediately
    template<class E>
    void Dispatch(const E& e) const {
        auto it = subs_.find(Type(typeid(E)));
        if (it == subs_.end()) return;
        for (auto& slot : it->second) slot.fn(&e);
    }
};

#endif // EVENT_H_