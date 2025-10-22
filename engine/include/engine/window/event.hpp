#ifndef EVENT_H_
#define EVENT_H_

#include <functional>
#include <algorithm>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine {

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

class EventHandler {
public:
    EventHandler() = default;

    virtual void OnEvent(const Event& event) = 0;
};

class EventEmitter {
public:
    struct Handle {
        std::size_t id{0};
        explicit operator bool() const { return id != 0; }
    };

    EventEmitter() = default;

    Handle Subscribe2(EventHandler* handler) {
        auto slot = Slot{ m_next_id++, handler };
        m_subs.push_back(slot);
        return Handle{ slot.id };
    }

    void Unsubscribe2(const Handle& h) {
        m_subs.erase(std::remove_if(m_subs.begin(), m_subs.end(),
                  [&](const Slot& s){ return s.id == h.id; }),
                  m_subs.end());
    }    
protected:
    void EmitEvent(const Event& event) {
        for (auto &sub : m_subs) {
            sub.handler->OnEvent(event);
        }
    }

private:
    struct Slot { std::size_t id; EventHandler* handler; };
    std::vector<Slot> m_subs;
    std::size_t m_next_id = 1;
};

}

#endif // EVENT_H_