#pragma once

#include <cstdint>


/// \brief choose entity representation - 32bit should generally be enough
#ifdef _32_BIT_ENTITY
using EntityId = std::uint32_t;
using EntityIndex = std::uint16_t;
#elif _64_BIT_ENTITY
using EntityId = std::uint64_t;
using EntityIndex = std::uint32_t;
#endif