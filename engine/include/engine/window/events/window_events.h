#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "engine/window/event.hpp"

namespace Engine {

class WindowEvent : public Event {
public:
    WindowEvent() : Event(Event::EventCategory::WINDOW) {}

    inline const EventType GetType() const override { return EventType::WINDOW_CLOSE; }
};

class WindowResizeEvent : public WindowEvent {
public:
    WindowResizeEvent(unsigned int w, unsigned int h) : m_width(w), m_height(h) {}

    inline const EventType GetType() const override { return EventType::WINDOW_RESIZE; }

    inline const unsigned int GetWidth() const { return m_width; }
    inline const unsigned int GetHeight() const { return m_height; }
private:
    unsigned int m_width, m_height;
};

class WindowCloseEvent : public WindowEvent {
public:
    WindowCloseEvent() {}
};

}

#endif // WINDOW_EVENTS_H_