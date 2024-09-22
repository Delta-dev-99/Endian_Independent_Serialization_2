#pragma once

#include <dd99/eis2/io/buffer.hpp>



namespace dd99::eis2::io
{

    template <class OutIter>
    struct Output_To_Iterator
    {
        OutIter m_out;

        template <dd99::eis2::io::ConstBufferSequence T>
        void write(const T & buffers)
        {
            for (const auto & x : buffers)
                m_out = std::copy(x.begin(), x.end(), m_out);
        }

        template <dd99::eis2::io::ConstBuffer T>
        void write(const T & buffer)
        {
            m_out = std::copy(buffer.begin(), buffer.end(), m_out);
        }
    };

}
