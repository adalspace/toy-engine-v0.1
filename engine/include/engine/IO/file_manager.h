#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

namespace Engine {

class FileManager
{
public:
    FileManager();
    ~FileManager();

    static std::string read(const std::string &filename);
};

}

#endif // FILE_MANAGER_H