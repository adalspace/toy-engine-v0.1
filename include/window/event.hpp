#ifndef EVENT_H_
#define EVENT_H_

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>

class EventBus {
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
    Handle subscribe(F&& f) {
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
    void unsubscribe(const Handle& h) {
        auto it = subs_.find(h.type);
        if (it == subs_.end()) return;
        auto& vec = it->second;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                  [&](const Slot& s){ return s.id == h.id; }),
                  vec.end());
    }

    // Publish immediately
    template<class E>
    void publish(const E& e) const {
        auto it = subs_.find(Type(typeid(E)));
        if (it == subs_.end()) return;
        for (auto& slot : it->second) slot.fn(&e);
    }
};

// Optional RAII helper
struct ScopedSub {
    EventBus* bus{};
    EventBus::Handle h{};
    ScopedSub() = default;
    ScopedSub(EventBus& b, EventBus::Handle hh) : bus(&b), h(hh) {}
    ScopedSub(ScopedSub&& o) noexcept { *this = std::move(o); }
    ScopedSub& operator=(ScopedSub&& o) noexcept {
        if (this != &o) { reset(); bus = o.bus; h = o.h; o.bus = nullptr; }
        return *this;
    }
    ~ScopedSub(){ reset(); }
    void reset(){ if (bus && h) bus->unsubscribe(h); bus=nullptr; h={}; }
};

#endif // EVENT_H_