#pragma once

#include <optional>

namespace oxm {

/// Finds index of target in array using AVX2 instruction set
std::optional<uint32_t> FindUint8(const uint8_t* data, uint32_t size, uint8_t target);

}  // namespace oxm
