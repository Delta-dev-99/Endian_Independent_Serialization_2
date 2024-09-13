#pragma once



namespace dd99::eis2::io
{

    template <class T> concept ConstBuffer =
        requires(const T & t)
        {
            // std::convertible_to<typename T::value_type, const std::byte>;
            { t.data() } -> std::convertible_to<const std::byte *>;
            { t.size() } -> std::convertible_to<std::size_t>;
        };


    template <class T> concept ConstBufferSequence =
        ConstBuffer<typename std::iterator_traits<decltype(std::declval<const T &>().begin())>::value_type> &&
        requires(const T & t)
        {
            { t.begin() } -> std::forward_iterator;
            { t.end() } -> std::forward_iterator;
        };



    
    template <class T> concept MutableBuffer = ConstBuffer<T> &&
        requires(T & t)
        {
            // std::convertible_to<typename T::value_type, std::byte>;
            { t.data() } -> std::convertible_to<std::byte *>;
            { t.size() } -> std::convertible_to<std::size_t>;
        };

    template <class T> concept MutableBufferSequence = ConstBufferSequence<T> &&
        MutableBuffer<typename std::iterator_traits<decltype(std::declval<const T &>().begin())>::value_type>;





    // ConstBuffer subsumes MutableBuffer
    // This function should return a mutable buffer when possible
    template <ConstBuffer T>
    constexpr auto buffer(T & t)
    {
        if constexpr (std::convertible_to<T, std::span<std::byte>>)
            return std::span<std::byte>{t};
        else if constexpr (std::convertible_to<T, std::span<const std::byte>>)
            return std::span<const std::byte>{t};
        else if constexpr (MutableBuffer<T>)
            return std::span<std::byte>{t.data(), t.size()};
        else return std::span<const std::byte>{t.data(), t.size()};
    }

}
