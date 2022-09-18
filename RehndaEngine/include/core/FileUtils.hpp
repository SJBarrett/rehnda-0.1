#pragma once

#include <fstream>
#include <vector>

namespace Rehnda::FileUtils {
    std::vector<char> readFileAsBytes(const std::string &filename);
}