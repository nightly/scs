#pragma once

#include <vector>
#include <ostream>

namespace scs {

    template <typename T>
    std::ostream& operator<< (std::ostream& os, const std::vector<std::vector<T>>& vec) {
        for (const auto& inner : vec) {
            for (size_t i = 0; i < inner.size(); ++i) {
                os << inner[i];
                if (i != inner.size() - 1) {
                    os << ", ";
                }
            }
            os << "\n";
        }
        return os;
    }

}