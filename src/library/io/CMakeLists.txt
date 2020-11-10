add_library(io
    push_stream.cpp
    pop_stream.cpp
    buffer.cpp
)

target_include_directories(io
    PUBLIC
        $<BUILD_INTERFACE:${_io_include_dir}>
        $<INSTALL_INTERFACE:include/io>
)

target_compile_features(io PUBLIC cxx_std_17)