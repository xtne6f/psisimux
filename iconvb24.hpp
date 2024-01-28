#ifndef INCLUDE_ICONVB24_HPP
#define INCLUDE_ICONVB24_HPP

#include <stdint.h>
#include <vector>

void ConvertUcsCaptionToArib8(std::vector<uint8_t> &data, int (&ucsLatinModes)[8], std::vector<uint8_t> &work);

#endif // INCLUDE_ICONVB24_HPP
