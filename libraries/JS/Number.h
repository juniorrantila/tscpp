#include <Ty/Forward.h>

namespace JS {

struct Number {
    Number(double value)
        : m_value(value)
    {
    }

    Number operator+(Number other) const
    {
        return Number(m_value + other.m_value);
    }

    Number operator-(Number other) const
    {
        return Number(m_value + other.m_value);
    }

    bool operator<=(Number other) const
    {
        return m_value <= other.m_value;
    }

    bool operator==(Number other) const
    {
        return m_value == other.m_value;
    }

    ErrorOr<StringBuffer> toString() const;

private:
    double m_value { 0.0 };
};

using number = Number;

}

using JS::Number;
using JS::number;
