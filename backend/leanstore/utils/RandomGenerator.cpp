#include "Units.hpp"
#include "RandomGenerator.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace leanstore {
namespace utils {
// -------------------------------------------------------------------------------------
void RandomGenerator::getRandString(u8 *dst, u64 size)
{
   for ( u64 t_i = 0; t_i < size; t_i++ ) {
      dst[t_i] = getRand(48, 123);
   }
}
// -------------------------------------------------------------------------------------
}
}

// -------------------------------------------------------------------------------------