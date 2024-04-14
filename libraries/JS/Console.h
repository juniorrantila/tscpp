#include <Core/File.h>

namespace JS {

struct Console {
    template <typename... Args>
    void log(Args... args) const
    {
        auto buf = StringBuffer();
        MUST(buffer(buf, args...));
        Core::File::stdout().writeln(buf.view()).ignore();
    }

    Console const* operator->() const { return this; }

private:
    template <typename... Args>
        requires (sizeof...(Args) > 1)
    ErrorOr<usize> buffer(StringBuffer& buf, Args const&... args) const
    {
        ErrorOr<usize> results[] = {
            buffer(buf, args)...
        };
        usize size = 0;
        for (auto& result : results)
            size += TRY(result);
        return size;
    }

    template <typename T>
    ErrorOr<usize> buffer(StringBuffer& buffer, T const& value) const
    {
        if constexpr (requires { value.toString(); }) {
            auto buf = TRY(value.toString());
            return TRY(buffer.write(buf.view()));
        } else {
            return TRY(buffer.write(value));
        }
    }
} inline console;

}

using JS::console;
