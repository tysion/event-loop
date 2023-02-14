#include "simd_algorithm.h"

#include <immintrin.h>

namespace oxm {

std::optional<uint32_t> FindUint8(const uint8_t* data, const uint32_t size, const uint8_t target) {
  const __m256i target_vec = _mm256_set1_epi8(target);
  const uint32_t avx2_end = (size & ~31);

  for (uint32_t i = 0; i < avx2_end; i += 32) {
    const __m256i data_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&data[i]));
    const int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(target_vec, data_vec));

    if (mask != 0) {
      return i + __builtin_ctz(mask);
    }
  }

  for (uint32_t i = avx2_end; i < size; ++i) {
    if (data[i] == target) {
      return i;
    }
  }

  return std::nullopt;
}

}  // namespace oxm