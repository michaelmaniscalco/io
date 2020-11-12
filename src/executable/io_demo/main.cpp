#include <cstddef>
#include <iostream>
#include <memory>
#include <chrono>
#include <mutex>
#include <cstdint>
#include <queue>
#include <fstream>
#include <iomanip>
#include <optional>

#include <library/io.h>


namespace 
{
    // test settings
    using integer_type = std::uint64_t;
    static auto constexpr num_integers_to_push = (1ull << 23);
    static auto constexpr num_bits_per_push = 32;
}


//=============================================================================
template <typename OutputHandler, typename InputHandler>
auto stream_push_pop_test
(
    // generic function which will write data any output stream 
    // and then read back that data from any input stream
    OutputHandler outputHandler,
    InputHandler inputHandler,
    std::function<maniscalco::buffer()> customAllocationHandler = nullptr
) -> std::optional<std::tuple<std::chrono::nanoseconds, std::chrono::nanoseconds>>
{
    using namespace maniscalco;

    io::push_stream pushStream({outputHandler,
            customAllocationHandler});
    auto push_start = std::chrono::system_clock::now();
    // run test - push 0 to num_integers_to_push into the stream
    for (auto i = 0ull; i < num_integers_to_push; ++i)
        pushStream.push(i, num_bits_per_push);
    pushStream.flush();
    auto push_end = std::chrono::system_clock::now();

    // vaildate - pop those numbers from the stream
    auto success = true;
    io::pop_stream popStream({inputHandler});
    auto pop_start = std::chrono::system_clock::now();
    auto i = 0ull;
    for (; ((success) && (i < num_integers_to_push)); ++i)
        success = ((success) &&  (popStream.pop(num_bits_per_push) == i));
    auto pop_end = std::chrono::system_clock::now();

    if (success)
        return std::make_tuple(push_end - push_start, pop_end - pop_start);
    std::cout << "Test failed at integer " << i << std::endl;
    return std::nullopt;
}


//=============================================================================
auto memory_stream_test
(
    // stream to memory
    std::function<maniscalco::buffer()> optionalCustomBufferAllocationHook = nullptr
)
{
    using namespace maniscalco;

    // in memory stream
    std::deque<std::tuple<buffer, std::size_t>> output;

    return stream_push_pop_test(
            [&](buffer data, std::size_t size) // output buffer to our queue
            {
                output.emplace_back(std::make_tuple(std::move(data), size));
            },
            [&]() // retreive data from our queue
            {
                auto ret = std::move(output.front());
                output.pop_front();
                return ret;
            },
            optionalCustomBufferAllocationHook);
}


//=============================================================================
auto file_stream_test
(
    // stream to file
    std::function<maniscalco::buffer()> optionalCustomBufferAllocationHook = nullptr
)
{
    using namespace maniscalco;
    std::fstream file("/tmp/test.dat", std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

    return stream_push_pop_test(
            [&](buffer data, std::size_t numBits) // output data to our file
            {
                auto numBitsToWrite = (std::uint32_t)numBits;
                auto numBytesToWrite = ((numBitsToWrite + 7) >> 3);
                file.write(reinterpret_cast<char const *>(&numBitsToWrite), sizeof(std::uint32_t));
                file.write(reinterpret_cast<char const *>(data.data()), numBytesToWrite);
            },
            [&, seekStart = true]() mutable // retreive data from our file
            {
                if (seekStart)
                {
                    seekStart = false; // hack to ensure start at beginning of file
                    file.seekg(0);
                }
                std::uint32_t numBitsToRead;
                file.read(reinterpret_cast<char *>(&numBitsToRead), sizeof(std::uint32_t));
                auto numBytesToRead = ((numBitsToRead + 7) >> 3);
                buffer data(numBitsToRead);
                file.read(reinterpret_cast<char *>(data.data()), numBytesToRead);
                return std::make_tuple(std::move(data), numBitsToRead);
            },
            optionalCustomBufferAllocationHook);
}


//=============================================================================
template <typename T>
void benchmark_test
(
    // execute and log results of specified test
    T test,
    std::function<maniscalco::buffer()> optionalCustomBufferAllocationHook = nullptr
)
{
    auto result = test(optionalCustomBufferAllocationHook);

    std::cout << "\tsuccess = " << std::boolalpha << result.has_value() << std::endl;
    if (result.has_value())
    {
        static auto constexpr megabytesProcessed = (((num_integers_to_push * num_bits_per_push) / (1 << 20)) / 8);
        auto [pushElapsed, popElapsed] = *result;
        auto elapsedPushInSec = ((double)std::chrono::duration_cast<std::chrono::microseconds>(pushElapsed).count() / 1000000);
        std::cout << "\tPush: total = " << megabytesProcessed << " MB, elapsed = " << 
                elapsedPushInSec << " sec,  " << (megabytesProcessed / elapsedPushInSec) << " MB/sec" << std::endl;

        auto elapsedPopInSec = ((double)std::chrono::duration_cast<std::chrono::microseconds>(popElapsed).count() / 1000000);
        std::cout << "\tPop: total = " << megabytesProcessed << " MB, elapsed = " << 
                elapsedPopInSec << " sec,  " << (megabytesProcessed / elapsedPopInSec) << " MB/sec" << std::endl;
    }
}


//=============================================================================
int main
(
    int, 
    char const **
)
{
    // demonstrate basic memory stream
    std::cout << "Memory stream test - default buffer size:" << std::endl;
    benchmark_test(&memory_stream_test);

    // demonstrate basic memory stream with non default buffer sizes
    std::cout << "Memory stream test - custom 1MB buffer size:" << std::endl;
    benchmark_test(&memory_stream_test, [](){return maniscalco::buffer((1 << 20) * 8);});

    // demonstrate basic file stream
    std::cout << "File stream test - default buffer size:" << std::endl;
    benchmark_test(&file_stream_test);

    // demonstrate basic file stream wtih non default buffer sizes
    std::cout << "File stream test - custom 1MB buffer size:" << std::endl;
    benchmark_test(&file_stream_test, [](){return maniscalco::buffer((1 << 20) * 8);});

    // demonstate custom buffer allocator - in this case using file stream
    std::cout << "File stream test - buffer w/ custom alloaction:" << std::endl;
    benchmark_test(&file_stream_test, 
        []()
        {
            static auto constexpr size = ((1 << 16) * 8);
            auto buffer = maniscalco::buffer(new std::uint8_t[size], size, [](auto * p){delete [] p;});
            std::fill(buffer.data(), buffer.data() + (buffer.capacity() + 7) / 8, 0x00);
            return buffer;
        });

    return 0;
}
