#include <string>
#include <cstdint>

class Code
{
public:
    Code();
    ~Code() = default;

    uint16_t dest(const std::string& mnemonic);
    uint16_t comp(const std::string& mnemonic);
    uint16_t jump(const std::string& mnemonic);
};