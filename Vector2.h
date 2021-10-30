#include <Windows.h>

class VectorXY
{
public:
    int x;
    int y;

    inline VectorXY(int X = 0, int Y = 0)
    {
        x = X;
        y = Y;
    }

    inline VectorXY(COORD coord)
    {
        x = coord.X;
        y = coord.Y;
    }

    inline bool operator!=(const VectorXY &right) { return (x != right.x || y != right.y); }

    inline bool operator==(const VectorXY &right) { return (x == right.x && y == right.y); }

    inline void operator*=(const int &right)
    {
        x *= right;
        y *= right;
    }

    inline VectorXY operator*(const int &right)
    {
        VectorXY temp;
        temp.x = x * right;
        temp.y = y * right;
        return temp;
    }

    inline VectorXY operator+(const VectorXY &right)
    {
        VectorXY temp;
        temp.x = x + right.x;
        temp.y = y + right.y;
        return temp;
    }

    inline void operator+=(const VectorXY &right)
    {
        x += right.x;
        y += right.y;
    }

    inline VectorXY operator-(const VectorXY &right)
    {
        VectorXY temp;
        temp.x = x - right.x;
        temp.y = y - right.y;
        return temp;
    }

    inline void operator-=(const VectorXY &right)
    {
        x -= right.x;
        y -= right.y;
    }

    inline VectorXY operator/(const int &right)
    {
        VectorXY temp;
        temp.x = x / right;
        temp.y = y / right;
        return temp;
    }

    inline void operator/=(const int &right)
    {
        x /= right;
        y /= right;
    }

    inline operator COORD()
    {
        COORD coord = { x,y };
        return coord;
    }
};
typedef VectorXY XY;

inline VectorXY operator-(const VectorXY& right)
{
    return VectorXY(-right.x, -right.y);
}