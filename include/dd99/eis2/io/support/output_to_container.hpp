#pragma once

#include <dd99/eis2/io/buffer.hpp>



namespace dd99::eis2::io
{

    namespace internal
    {
        template <class T> concept Container = 
            std::convertible_to<std::byte, typename std::remove_reference_t<T>::value_type> &&
            requires(T & t)
            {
                t.empty();
                t.size();
                t.begin();
                t.end();
            };
    }


    template <internal::Container ContainerT>
    struct Output_To_Container
    {
        ContainerT m_container{};

        template <dd99::eis2::io::ConstBufferSequence T>
        void write(const T & buffers)
        {
            std::size_t size = 0;
            for (const auto & x : buffers)
                size += x.size();

            m_container.resize(m_container.size() + size);
            auto out_iter = m_container.end() - size;

            for (const auto & x : buffers)
                out_iter = std::copy(x.begin(), x.end(), out_iter);
        }

        template <dd99::eis2::io::ConstBuffer T>
        void write(const T & buffer)
        {
            m_container.resize(m_container.size() + buffer.size());
            std::copy(buffer.begin(), buffer.end(), m_container.end() - buffer.size());
        }
    };

}
