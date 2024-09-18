#pragma once

#include <span>
#include <cstddef>
#include <dd99/eis2/io/buffer.hpp>



namespace dd99::eis2::io
{

    // assumes we are writing bytes
    // struct Output_Device_Base
    // {
    //     // write single char
    //     void write(std::byte);
        
    //     // write a buffer
    //     void write(std::span<const std::byte>);

    //     // write multiple buffers (scatter/gather)
    //     void write(std::span<const std::span<const std::byte>>);
    // };

    // struct Async_Output_Device_Base
    // {
    //     auto async_write(std::byte);

    //     auto async_write(std::span<const std::byte>);

    //     void async_write(std::span<const std::span<const std::byte>>);
    // };


    template <class T> concept OutputDevice =
        requires(T & t)
        {
            // { t.write(std::byte{}) };
            t.write(std::span<const std::byte>{});
        };

    template <class T> concept ScatterGatherOutputDevice = OutputDevice<T> &&
        requires(T & t)
        {
            t.write(std::initializer_list<const std::span<const std::byte>>{});
        };


    // template <class T> concept AsyncOutputDevice =
    //     requires(T & t)
    //     {
    //         // { t.async_write(std::byte{}) };
    //         t.async_write(std::span<const std::byte>{});
    //     };

    // template <class T> concept AsyncScatterGatherOutputDevice = OutputDevice<T> &&
    //     requires(T & t)
    //     {
    //         t.async_write(std::initializer_list<const std::span<const std::byte>>{});
    //     };



    template <OutputDevice Out, ConstBuffer CBuffer>
    constexpr auto write(Out & output, const CBuffer buf)
    { output.write(buffer(buf)); }

    template <OutputDevice Out, ConstBufferSequence CBufferSeq>
    constexpr auto write(Out & output, const CBufferSeq & buf_seq)
    {
        if constexpr (ScatterGatherOutputDevice<Out>)
            output.write(buf_seq);
        else
            for (const auto & buf : buf_seq)
                output.write(buffer(buf));
    }

    template <OutputDevice Out, ConstBuffer ... CBuffers>
    constexpr auto write(Out & output, const CBuffers ... buffers)
    {
        if constexpr (ScatterGatherOutputDevice<Out>)
            output.write(std::initializer_list<const std::span<const std::byte>>{buffers...});
        else
            (output.write(buffers), ...);
    }

    template <OutputDevice Out, ConstBuffer ... CBuffers>
    constexpr auto write(Out & output, const std::tuple<CBuffers...> & buffers)
    {
        std::apply([&](const auto & ... buf){
            write(output, buf...);
        }, buffers);
    }

}
