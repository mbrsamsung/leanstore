#pragma once
#include "Units.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
#include <chrono>
#include <cmath>
// -------------------------------------------------------------------------------------
namespace leanstore
{
namespace utils
{
// -------------------------------------------------------------------------------------
u32 getBitsNeeded(u64 input);
// -------------------------------------------------------------------------------------
double calculateMTPS(std::chrono::high_resolution_clock::time_point begin, std::chrono::high_resolution_clock::time_point end, u64 factor);
// -------------------------------------------------------------------------------------
void pinThisThreadRome();
void pinThisThreadRome(const u64 t_i);
void pinThisThread(const u64 t_i);
// -------------------------------------------------------------------------------------
void printBackTrace();
// -------------------------------------------------------------------------------------
}  // namespace utils
}  // namespace leanstore
