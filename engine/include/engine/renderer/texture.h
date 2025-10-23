#ifndef TEXTURE_H_
#define TEXTURE_H_
#include <string>
#include <memory>

namespace Core {

class Texture {
public:
    Texture() : m_id(0) {}
    static std::unique_ptr<Texture> LoadFile(const std::string& filename);
public:
    [[nodiscard]] unsigned int GetID() const { return m_id; }
private:
    unsigned int m_id;
};

}

#endif // TEXTURE_H_
