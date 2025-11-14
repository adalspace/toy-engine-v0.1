#ifndef CORE_ARRAY_H_
#define CORE_ARRAY_H_

#define ARRAY_INITIAL_CAPACITY 2

namespace Core {

template<typename Item, typename Size = unsigned int>
class Array {
public:
    Array() {
        m_size = 0;
        m_capacity = ARRAY_INITIAL_CAPACITY;
        m_items = static_cast<Item*>(::operator new[](m_capacity * sizeof(Item)));
    }

    Array(Size initialCapacity) {
        m_size = 0;
        m_capacity = initialCapacity;
        m_items = static_cast<Item*>(::operator new[](m_capacity * sizeof(Item)));
    }

    ~Array() {
        for (unsigned int i = 0; i < m_size; ++i) {
            m_items[i].~Item();
        }
        ::operator delete[](m_items);
    }

    Array(Array&& other) {
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_items = other.m_items;

        other.m_size = 0;
        other.m_capacity = 0;
        other.m_items = nullptr;
    }

    Array& operator=(Array&& other) noexcept {
        if (this != &other) {
            // Destroy current contents
            for (unsigned int i = 0; i < m_size; ++i)
                m_items[i].~Item();
            ::operator delete[](m_items);

            // Move from other
            m_items = other.m_items;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.m_items = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    Array(const Array&) = delete;
public:
    inline const Size GetSize() const noexcept { return m_size; }

    Item* Begin() { return m_items; }
    const Item* Begin() const { return m_items; }

    Item& Back() { assert(m_size > 0 && "Calling back() on empty array!"); return m_items[m_size - 1]; }
    const Item& Back() const { assert(m_size > 0 && "Calling back() on empty array!"); return m_items[m_size - 1]; }

    Item* End() { return m_items + m_size; }
    const Item* End() const { return m_items + m_size; }

    inline const bool Empty() const { return m_size == 0; }
public:
    template<typename... Args>
    void EmplaceBack(Args&&... args) {
        _ensureSize(m_size + 1);
        new (&m_items[m_size++]) Item(std::forward<Args>(args)...);
    }

    void PushBack(const Item& item) {
        _ensureSize(m_size + 1);
        new (&m_items[m_size++]) Item(item);
    }

    void PushBack(Item&& item) {
        _ensureSize(m_size + 1);
        new (&m_items[m_size++]) Item(std::move(item));
    }
private:
    void _ensureSize(Size size) {
        if (size > m_capacity) {
            _extend(static_cast<Size>(m_capacity + (m_capacity / 2)));
        }
    }

    void _extend(Size newSize) {
        auto newItems = static_cast<Item*>(::operator new[](newSize * sizeof(Item)));

        std::uninitialized_move(
            std::make_move_iterator(m_items),
            std::make_move_iterator(m_items + m_size),
            newItems
        );

        for (unsigned int i = 0; i < m_size; ++i) {
            m_items[i].~Item();
        }

        ::operator delete[](m_items);

        m_items = newItems;
        m_capacity = newSize;
    }
private:
    Item* m_items;
    Size m_size;
    Size m_capacity;
};

}

#endif // CORE_ARRAY_H_