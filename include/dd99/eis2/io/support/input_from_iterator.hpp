#pragma once

#include <dd99/eis2/io/output_device.hpp>
#include <iterator>



namespace dd99::eis2::io
{

    template <class InputIterT>
    struct Input_From_Iterator
    {

        InputIterT m_iter;

        template <dd99::eis2::io::MutableBufferSequence T>
        void read(const T & buffers)
        {
            for (const auto & x : buffers)
            {
                std::copy_n(m_iter, x.size(), x.begin());
                std::advance(m_iter, x.size());
            }
        }

        template <dd99::eis2::io::MutableBuffer T>
        void read(const T & buffer)
        {
            std::copy_n(m_iter, buffer.size(), buffer.begin());
            std::advance(m_iter, buffer.size());
        }

        template <dd99::eis2::io::OutputDevice T>
        void read(T & out, std::size_t n)
        {
            std::vector<std::byte> buf;
            buf.resize(n);
            read(buffer(buf));
            dd99::eis2::io::write(out, buffer(buf));
        }


    };

}
