#pragma once

#include <cmath>
#include <ostream>
#include <type_traits>

namespace physenv {

namespace details {

template <typename T>
concept Vectorisable = std::is_arithmetic_v<T>;

template <Vectorisable T>
class Vector2 {
  public:
    T x{};
    T y{};

    constexpr Vector2() = default;

    constexpr Vector2(T x_, T y_) : x(x_), y(y_) {}

    template <typename U>
    explicit operator Vector2<U>() const {
        return Vector2<U>(static_cast<U>(x), static_cast<U>(y));
    }

    constexpr T mag() const
        requires std::is_floating_point_v<T>
    {
        return std::hypot(x, y);
    }
    constexpr Vector2 norm() const
        requires std::is_floating_point_v<T>
    {
        return *this / this->mag();
    }
    constexpr Vector2 abs() const { return {std::abs(x), std::abs(y)}; }
    constexpr T       dot(const Vector2& rhs) const { return x * rhs.x + y * rhs.y; }
    constexpr T       cross(const Vector2& rhs) const { return x * rhs.y - y * rhs.x; }
    constexpr T       distToLine(const Vector2& p1, const Vector2& p2) const
        requires std::is_floating_point_v<T>
    {
        Vector2 line  = p2 - p1;
        Vector2 diff1 = *this - p1;
        if (diff1.dot(line) < 0) return diff1.mag();
        Vector2 diff2 = *this - p2;
        if (diff2.dot(line) > 0) return diff2.mag();
        return std::abs(line.cross(diff1)) / line.mag();
    }
    std::string toString() const {
        return '(' + std::to_string(x) + ", " + std::to_string(y) + ')';
    }

    // clang-format off
    constexpr Vector2& operator+=(const Vector2& obj) { x += obj.x; y += obj.y; return *this; }
    constexpr Vector2& operator-=(const Vector2& obj) { x -= obj.x; y -= obj.y; return *this; }
    constexpr Vector2& operator*=(T scale) { x *= scale; y *= scale; return *this; }
    constexpr Vector2& operator/=(T scale) { x /= scale; y /= scale; return *this; }
    // clang-format on

    constexpr friend Vector2 operator+(Vector2 lhs, const Vector2& rhs) { return lhs += rhs; }
    constexpr friend Vector2 operator-(Vector2 lhs, const Vector2& rhs) { return lhs -= rhs; }
    constexpr friend Vector2 operator*(Vector2 lhs, T scale) { return lhs *= scale; }
    constexpr friend Vector2 operator*(T scale, Vector2 rhs) { return rhs *= scale; }
    constexpr friend Vector2 operator/(Vector2 lhs, T scale) { return lhs /= scale; }
    bool                     operator==(const Vector2&) const = default;

    friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
        return os << v.x << " " << v.y;
    }
};

} // namespace details

using Vec2  = details::Vector2<double>;
using Vec2I = details::Vector2<int>;
using Vec2U = details::Vector2<unsigned>;
using Vec2F = details::Vector2<float>;

} // namespace physenv