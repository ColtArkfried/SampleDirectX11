// ------------------------------------------------------------------
// Å°ìÒéüå≥ç¿ïWÉNÉâÉX
// ------------------------------------------------------------------
#pragma once

// ------------------------------------------------------------------
// Å°namespace ark::utility
namespace ark
{
    namespace utility
    {
        template <typename T> class Point
        {
        public:

            Point(const T& _x = 0, const T& _y = 0): x(_x), y(_y) {}
            Point(const Point<T>& copy): x(copy.x), y(copy.y) {}

            const Point<T>& operator=(const Point<T>& copy)
            {
                x = copy.x;
                y = copy.y;
                return (*this);
            }

            void set(const T& _x = 0, const T& _y = 0)
            {
                x = _x;
                y = _y;
            }

            Point<T> operator+(const Point<T>& rhd) const
            {
                return Point<T>(x + rhd.x, y + rhd.y);
            }

            Point<T> operator-(const Point<T>& rhd) const
            {
                return Point<T>(x - rhd.x, y - rhd.y);
            }

            Point<T> operator*(const T& scale) const
            {
                return Point<T>(x*scale, y*scale);
            }

            Point<T> operator/(const T& scale) const
            {
                if(scale != 0) return Point<T>(x / scale, y / scale);
                else         return Point<T>(x, y);
            }

            const Point<T>& operator+=(const Point<T>& rhd)
            {
                x += rhd.x;
                y += rhd.y;
                return (*this);
            }

            const Point<T>& operator-=(const Point<T>& rhd)
            {
                x -= rhd.x;
                y -= rhd.y;
                return (*this);
            }

            const Point<T>& operator*=(const T& scale)
            {
                x *= scale;
                y *= scale;
                return (*this);
            }

            const Point<T>& operator/=(const T& scale)
            {
                if(scale != 0)
                {
                    x /= scale;
                    y /= scale;
                }
                return (*this);
            }

            void clamp(const T& min, const T& max)
            {
                x = x > max ? max : x < min ? min : x;
                y = y > max ? max : y < min ? min : y;
            }

            void clamp(const T& min_x, const T& max_x, const T& min_y, const T& max_y)
            {
                x = x > max_x ? max_x : x < min_x ? min_x : x;
                y = y > max_y ? max_y : y < min_y ? min_y : y;
            }

            T x;
            T y;
        };
    }
}