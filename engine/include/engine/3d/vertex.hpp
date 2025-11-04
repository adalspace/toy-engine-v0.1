#pragma once

#include <cstring>
#include <glm/glm.hpp>

#define VERTICES_INITIAL_CAPACITY 256
#define INDICES_INITIAL_CAPACITY 512

namespace Core {

struct Vertex {
public:
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv)
        : position(position), normal(normal), uv(uv) {}
    Vertex()
        : position(glm::vec3()), normal(glm::vec3()), uv(glm::vec2()) {}

    Vertex(const Vertex&) noexcept = default;
    Vertex(Vertex&&) noexcept = default;

    bool operator ==(Vertex const& o) const noexcept {
        return o.position == position
            && o.normal == normal
            && o.uv == uv;
    }
public:
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    
    friend class VertexHash;
};

struct VertexHash {
    size_t operator()(Vertex const& v) const noexcept {
        auto h = [](float f) -> size_t {
            uint32_t b;
            static_assert(sizeof(float) == sizeof(uint32_t));
            std::memcpy(&b, &f, sizeof(float));
            // splitmix64-like mixing (simple)
            uint64_t x = b;
            x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
            x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
            x ^= x >> 31;
            return static_cast<size_t>(x);
        };

        size_t res = 1469598103934665603ULL;
        res ^= h(v.position.x) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);
        res ^= h(v.position.y) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);
        res ^= h(v.position.z) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);

        res ^= h(v.normal.x) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);
        res ^= h(v.normal.y) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);
        res ^= h(v.normal.z) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);

        res ^= h(v.uv.x) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);
        res ^= h(v.uv.y) + 0x9e3779b97f4a7c15ULL + (res<<6) + (res>>2);
        return res;
    }
};

}