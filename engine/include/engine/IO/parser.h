#ifndef PARSER_H_
#define PARSER_H_

namespace Engine {

// Very fast OBJ/MTL line parser
class Parser {
private:
    char* m_sv;
public:
    Parser(char* sv) : m_sv(sv) {}
public:
    void SkipSpaces();
    char* TakeWord();
    float TakeFloat();
    int TakeInt();
    bool TakeFaceIndices(int& vi, int& ti, int& ni);
    char* TakeUntil(char d);
    int TakeIndex(int baseCount);
};

}

#endif // PARSER_H_