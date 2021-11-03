#pragma once


// rendering channel (aggregation of `Pass`/`Effect` s)
namespace rch
{


inline constexpr size_t lambert = 0b1;
inline constexpr size_t shadow = 0b10;
inline constexpr size_t solidOutline = 0b100;
inline constexpr size_t blurOutline = 0b1000;


}