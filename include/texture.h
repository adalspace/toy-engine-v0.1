#ifndef TEXTURE_H_
#define TEXTURE_H_
#include <string>
#include <memory>

class Texture {
private:
    unsigned int m_id;
private:
public:
    Texture() {}
    static std::unique_ptr<Texture> LoadFile(const std::string& filename);
public:
    inline const unsigned int GetID() const { return m_id; }
};

#endif // TEXTURE_H_
