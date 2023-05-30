#include "string_utilities.h"

#include <vector>
#include <sstream>
#include <algorithm>

namespace Syft {

  std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
      tokens.push_back(str.substr(start, end - start));
      start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
  }

  std::string trim(const std::string& str) {
    std::string trimmed_str = str;
    trimmed_str.erase(trimmed_str.begin(), std::find_if(trimmed_str.begin(), trimmed_str.end(), [](int ch) {
      return !std::isspace(ch);
    }));
    trimmed_str.erase(std::find_if(trimmed_str.rbegin(), trimmed_str.rend(), [](int ch) {
      return !std::isspace(ch);
    }).base(), trimmed_str.end());
    return trimmed_str;
  }
}