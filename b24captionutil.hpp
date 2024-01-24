#ifndef INCLUDE_B24_CAPTION_UTIL_HPP
#define INCLUDE_B24_CAPTION_UTIL_HPP

#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <vector>

#ifdef _WIN32
void LoadWebVttB24Caption(const wchar_t *path, std::vector<std::pair<int64_t, std::vector<uint8_t>>> &captionList);
#else
void LoadWebVttB24Caption(const char *path, std::vector<std::pair<int64_t, std::vector<uint8_t>>> &captionList);
#endif
uint16_t CalcCrc16Ccitt(const uint8_t *data, size_t len, uint16_t crc = 0);

#endif // INCLUDE_B24_CAPTION_UTIL_HPP
