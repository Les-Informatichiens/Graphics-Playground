//
// Created by Jonathan Richard on 2024-03-05.
//

#include "engine/util/UUID.h"
#include <random>

namespace util {

static std::random_device s_RandomDevice;
static std::mt19937_64 s_Engine(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

UUID::UUID()
    : m_UUID(s_UniformDistribution(s_Engine))
{
}

UUID::UUID(uint64_t uuid)
    : m_UUID(uuid)
{
}

bool UUID::operator==(const UUID& other) const
{
    return m_UUID == other.m_UUID;
}

bool UUID::operator!=(const UUID& other) const
{
    return m_UUID != other.m_UUID;
}

UUID::operator std::string() const
{
    return std::to_string(m_UUID);
}

std::ostream& operator<<(std::ostream& os, const UUID& uuid)
{
    os << std::to_string(uuid.m_UUID);
    return os;
}

} // namespace util

