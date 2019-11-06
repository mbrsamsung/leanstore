#pragma once
// -------------------------------------------------------------------------------------
#include "Units.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
#include <atomic>
// -------------------------------------------------------------------------------------
namespace leanstore {
// -------------------------------------------------------------------------------------
struct BufferFrame; // Forward declaration
// -------------------------------------------------------------------------------------
using SwipValue = u64;
template<typename T>
class Swip {
   // -------------------------------------------------------------------------------------
   static const u64 unswizzle_bit = u64(1) << 63;
   static const u64 unswizzle_mask = ~(u64(1) << 63);
   static_assert(unswizzle_bit == 0x8000000000000000, "");
   static_assert(unswizzle_mask == 0x7FFFFFFFFFFFFFFF, "");
public:
   u64 val;
   // -------------------------------------------------------------------------------------
   Swip() = default;
   Swip(BufferFrame *bf) : val(u64(bf)) {}
   template<typename T2>
   Swip(Swip<T2> &other) : val(other.val) {}
   // -------------------------------------------------------------------------------------
   bool operator==(const Swip &other) const
   {
      return (val == other.val);
   }
   // -------------------------------------------------------------------------------------
   bool isSwizzled()
   {
      return (val & unswizzle_mask);
   }
   u64 asInteger() { return val; }
   u64 asPageID() { return val & unswizzle_mask; }
   // -------------------------------------------------------------------------------------
   template<typename T2>
   void swizzle(T2 *bf)
   {
      val = u64(bf);
   }
   void unswizzle(PID pid) {
      val = pid | unswizzle_bit;
   }
   // -------------------------------------------------------------------------------------
   template<typename T2>
   Swip<T2> &cast() {
      return *reinterpret_cast<Swip<T2>*>(this);
   }
};
// -------------------------------------------------------------------------------------
}
// -------------------------------------------------------------------------------------