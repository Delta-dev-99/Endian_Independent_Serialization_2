#pragma once

#include <dd99/eis2/io/buffer.hpp>
#include <dd99/eis2/io/output_device.hpp>
#include <vector>



namespace dd99::eis2::io
{

    template <class T> concept InputDevice =
        requires(T & t, std::span<std::byte> buf)
        {
            t.read(buf);
            // t.read_some(buf);
        };

    template <class T> concept ScatterGatherInputDevice = InputDevice<T> &&
        requires (T & t, std::initializer_list<const std::span<std::byte>> buf_seq)
        {
            t.read(buf_seq);
        };


    // template <class T> concept AsyncInputDevice =
    //     requires(T & t, std::span<std::byte> buf)
    //     {
    //         t.async_read(buf);
    //     };

    // template <class T> concept ScatterGatherAsyncInputDevice =
    //     requires(T & t, std::initializer_list<const std::span<std::byte>> buf_seq)
    //     {
    //         t.async_read(buf_seq);
    //     };



    template <InputDevice InputT, MutableBuffer Buffer>
    constexpr auto read(InputT & input, const Buffer buf)
    { input.read(buffer(buf)); }

    template <InputDevice InputT, MutableBufferSequence BufferSeq>
    constexpr auto read(InputT & input, const BufferSeq & buf_seq)
    {
        if constexpr (ScatterGatherInputDevice<InputT>)
            input.read(buf_seq);
        else
            for (const auto & buf : buf_seq)
                input.read(buffer(buf));
    }

    template <InputDevice InputT, MutableBuffer ... Buffers>
    constexpr auto read(InputT & input, const Buffers ... buffers)
    {
        if constexpr (ScatterGatherInputDevice<InputT>)
            input.read(std::initializer_list<const std::span<std::byte>>{buffers...});
        else
            (input.read(buffers), ...);
    }

    template <InputDevice InputT, MutableBuffer ... Buffers>
    constexpr auto read(InputT & input, const std::tuple<Buffers...> & buffers)
    {
        std::apply([&](const auto & ... buf){
            read(input, buf...);
        }, buffers);
    }

    template <InputDevice InputT, OutputDevice OutT>
    constexpr auto read(InputT & input, OutT & output, std::size_t n)
    {
        if constexpr ( requires { input.read(output, n); } )
            return input.read(output, n);
        else
        {
            std::vector<std::byte> buf;
            buf.resize(n);
            dd99::eis2::io::read(input, buffer(buf));
            dd99::eis2::io::write(output, buffer(buf));
        }
    }


}
