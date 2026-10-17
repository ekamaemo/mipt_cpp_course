#pragma once

#include <string>

namespace nano_edr {

std::string ToLower(const std::string& str);

bool EqualsIgnoreCase(const std::string& line_a, const std::string& line_b);
bool ContainsIgnoreCase(const std::string& line, const std::string& needle);
bool EndsWithIgnoreCase(const std::string& line, const std::string& suffix);

} 