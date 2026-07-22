#ifndef CORE_ARRAY_H_
#define CORE_ARRAY_H_

#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef ARRAY_INITIAL_CAPACITY
#define ARRAY_INITIAL_CAPACITY 2
#endif

namespace Core {

template<typename Item, typename Size = unsigned int>
class Array
{
    static_assert(
        std::is_integral_v<Size> && std::is_unsigned_v<Size>,
        "Array Size must be an unsigned integer type"
    );

private:
    using Allocator = std::allocator<Item>;
    using AllocatorTraits = std::allocator_traits<Allocator>;

public:
    Array()
        : Array(static_cast<Size>(ARRAY_INITIAL_CAPACITY))
    {
    }

    explicit Array(Size initialCapacity)
        : m_items(Allocate(initialCapacity)),
          m_size(0),
          m_capacity(initialCapacity)
    {
    }

    ~Array()
    {
        Release();
    }

    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;

    Array(Array&& other) noexcept
        : m_items(std::exchange(other.m_items, nullptr)),
          m_size(std::exchange(other.m_size, 0)),
          m_capacity(std::exchange(other.m_capacity, 0))
    {
    }

    Array& operator=(Array&& other) noexcept
    {
        if (this == &other)
            return *this;

        Release();

        m_items = std::exchange(other.m_items, nullptr);
        m_size = std::exchange(other.m_size, 0);
        m_capacity = std::exchange(other.m_capacity, 0);

        return *this;
    }

public:
    [[nodiscard]]
    Size GetSize() const noexcept
    {
        return m_size;
    }

    [[nodiscard]]
    Size GetCapacity() const noexcept
    {
        return m_capacity;
    }

    [[nodiscard]]
    bool Empty() const noexcept
    {
        return m_size == 0;
    }

    Item* Begin() noexcept
    {
        return m_items;
    }

    const Item* Begin() const noexcept
    {
        return m_items;
    }

    Item* End() noexcept
    {
        return m_size == 0 ? m_items : m_items + m_size;
    }

    const Item* End() const noexcept
    {
        return m_size == 0 ? m_items : m_items + m_size;
    }

    Item& Back()
    {
        assert(m_size > 0 && "Calling Back() on an empty Array");
        return m_items[m_size - 1];
    }

    const Item& Back() const
    {
        assert(m_size > 0 && "Calling Back() on an empty Array");
        return m_items[m_size - 1];
    }

    Item& operator[](Size index)
    {
        assert(index < m_size && "Array index out of bounds");
        return m_items[index];
    }

    const Item& operator[](Size index) const
    {
        assert(index < m_size && "Array index out of bounds");
        return m_items[index];
    }

    Item* Data() noexcept
    {
        return m_items;
    }

    const Item* Data() const noexcept
    {
        return m_items;
    }

public:
    template<typename... Args>
    Item& EmplaceBack(Args&&... args)
    {
        EnsureSpaceForOneMore();

        Item* newItem = m_items + m_size;

        Allocator allocator;
        AllocatorTraits::construct(
            allocator,
            newItem,
            std::forward<Args>(args)...
        );

        ++m_size;
        return *newItem;
    }

    template<
        typename T = Item,
        std::enable_if_t<std::is_copy_constructible_v<T>, int> = 0
    >
    void PushBack(const Item& item)
    {
        EmplaceBack(item);
    }

    void PushBack(Item&& item)
    {
        EmplaceBack(std::move(item));
    }

    void Reserve(Size requestedCapacity)
    {
        if (requestedCapacity > m_capacity)
            Extend(requestedCapacity);
    }

    void Clear() noexcept
    {
        DestroyItems(m_items, m_size);
        m_size = 0;
    }

private:
    void EnsureSpaceForOneMore()
    {
        if (m_size == std::numeric_limits<Size>::max())
            throw std::length_error("Core::Array reached its maximum size");

        EnsureCapacity(static_cast<Size>(m_size + 1));
    }

    void EnsureCapacity(Size requiredCapacity)
    {
        if (requiredCapacity <= m_capacity)
            return;

        Size newCapacity = m_capacity;

        if (newCapacity == 0)
        {
            newCapacity = static_cast<Size>(ARRAY_INITIAL_CAPACITY);

            if (newCapacity == 0)
                newCapacity = 1;
        }

        while (newCapacity < requiredCapacity)
        {
            Size growth = static_cast<Size>(newCapacity / 2);

            if (growth == 0)
                growth = 1;

            const Size maximum = std::numeric_limits<Size>::max();

            if (newCapacity > maximum - growth)
            {
                newCapacity = requiredCapacity;
                break;
            }

            newCapacity = static_cast<Size>(newCapacity + growth);
        }

        Extend(newCapacity);
    }

    void Extend(Size newCapacity)
    {
        assert(newCapacity >= m_size);

        Item* newItems = Allocate(newCapacity);
        Size constructedItems = 0;

        Allocator allocator;

        try
        {
            for (; constructedItems < m_size; ++constructedItems)
            {
                AllocatorTraits::construct(
                    allocator,
                    newItems + constructedItems,
                    std::move_if_noexcept(m_items[constructedItems])
                );
            }
        }
        catch (...)
        {
            DestroyItems(newItems, constructedItems);
            Deallocate(newItems, newCapacity);
            throw;
        }

        DestroyItems(m_items, m_size);
        Deallocate(m_items, m_capacity);

        m_items = newItems;
        m_capacity = newCapacity;
    }

    void Release() noexcept
    {
        DestroyItems(m_items, m_size);
        Deallocate(m_items, m_capacity);

        m_items = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    static Item* Allocate(Size capacity)
    {
        if (capacity == 0)
            return nullptr;

        Allocator allocator;

        return AllocatorTraits::allocate(
            allocator,
            static_cast<std::size_t>(capacity)
        );
    }

    static void Deallocate(Item* items, Size capacity) noexcept
    {
        if (items == nullptr)
            return;

        Allocator allocator;

        AllocatorTraits::deallocate(
            allocator,
            items,
            static_cast<std::size_t>(capacity)
        );
    }

    static void DestroyItems(Item* items, Size count) noexcept
    {
        if (items == nullptr)
            return;

        Allocator allocator;

        while (count > 0)
        {
            --count;
            AllocatorTraits::destroy(allocator, items + count);
        }
    }

private:
    Item* m_items = nullptr;
    Size m_size = 0;
    Size m_capacity = 0;
};

} // namespace Core

#endif // CORE_ARRAY_H_
