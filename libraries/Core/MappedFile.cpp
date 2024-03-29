#include "./MappedFile.h"

#include <Ty/System.h>
#include <Ty/Defer.h>

namespace Core {

ErrorOr<MappedFile> MappedFile::open(StringView path, Mode mode)
{
    auto path_buffer = TRY(StringBuffer::create_fill(path, "\0"sv));
    return TRY(open(path_buffer.view().data(), mode));
}

ErrorOr<MappedFile> MappedFile::open(c_string path, Mode mode)
{
    int m = 0;
    switch(mode) {
        case ModeRead: m = O_RDONLY; break;
        case ModeWrite: m = O_WRONLY; break;
        case ModeReadWrite: m = O_RDWR; break;
    }

    auto fd = TRY(System::open(path, m));
    auto should_close_file = true;
    Defer close_file = [&] {
        if (should_close_file)
            System::close(fd).ignore();
    };
    auto file_stat = TRY(System::fstat(fd));
    if (!file_stat.is_regular())
        return Error::from_string_literal(
            "file is not a regular file");
    u32 size = file_stat.size();
    auto* data = TRY(System::mmap(size, PROT_READ | PROT_WRITE, mode == ModeRead ? MAP_PRIVATE : MAP_SHARED, fd));
    should_close_file = false;
    return MappedFile(data, size, fd);
}

MappedFile::~MappedFile()
{
    if (is_valid()) {
        System::munmap((void*)m_data, m_size).ignore();
        System::close(m_fd).ignore();
        invalidate();
    }
}

}
