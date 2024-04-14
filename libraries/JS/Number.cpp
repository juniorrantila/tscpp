#include "./Number.h"

#include <Ty/StringBuffer.h>
#include <Ty/ErrorOr.h>

namespace JS {

ErrorOr<StringBuffer> Number::toString() const
{
    return TRY(StringBuffer::create_fill(m_value));
}

}
