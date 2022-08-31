#pragma once

#include <named_type.hpp>
#include <gsl/pointers>

namespace Rehnda {
     using Pixels = fluent::NamedType<int32_t, struct PixelsTag, fluent::Addable, fluent::Subtractable, fluent::Comparable, fluent::Multiplicable>;

     template<typename T>
     using Owner = gsl::owner<T>;
}