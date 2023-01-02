add_library(io
    push_stream.cpp
    pop_stream.cpp
    buffer.cpp
)


target_link_libraries(io
    common)

target_include_directories(io
    PUBLIC
        $<BUILD_INTERFACE:${_io_include_dir}>
        $<INSTALL_INTERFACE:include/io>
)
