#pragma once
#include <string_view>
#include <utility>
#include <cstdint>

std::pair<char*, size_t> ConvertToUTF8(std::string_view input);
