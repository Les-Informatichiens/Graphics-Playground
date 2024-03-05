//
// Created by Jonathan Richard on 2024-03-05.
//

#pragma once

#include <string>
#include <cstdint>

namespace util {

class UUID
{
public:
    static UUID generate() { return {}; }

    UUID();
    explicit UUID(uint64_t uuid);
    UUID(const UUID&) = default;

    operator uint64_t() const { return m_UUID; }
    operator std::string() const;
    // all needed operator overloads
    bool operator==(const UUID& other) const;
    bool operator!=(const UUID& other) const;

    // ostream
    friend std::ostream& operator<<(std::ostream& os, const UUID& uuid);

private:
    uint64_t m_UUID;
};

} // namespace util

namespace std {
    template <typename T> struct hash;

    template<>
    struct hash<util::UUID>
    {
        std::size_t operator()(const util::UUID& uuid) const
        {
            return (uint64_t)uuid;
        }
    };


}