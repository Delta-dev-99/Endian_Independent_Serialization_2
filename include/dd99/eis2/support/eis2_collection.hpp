#pragma once

#include <dd99/eis2/serialize.hpp>
#include <iterator>
#include <cstdint>
#include <limits>

// serialization and deserialization of collections (containers, views, ranges, etc)


namespace dd99::eis2
{

    namespace internal
    {
        // a collection of serializable members
        // NOTE: This concept is not the same as internal::Collection (without underscore)
        template <class T>
        concept Collection_ = Serializable<typename std::remove_reference_t<T>::value_type> &&
        requires (const T & t, std::size_t n)
        {
            t.empty();
            t.size();
            // t.resize(n);
            t.begin();
            t.end();
        };
    }


    template <internal::Collection_ T>
    struct EIS2_Traits<T>
    {
        using value_type = std::remove_reference_t<T>;
        using staging_type = std::uint32_t;
        using element_type = typename value_type::value_type;

        static constexpr auto is_collection = true;
        static constexpr auto is_contiguous = std::contiguous_iterator<typename value_type::iterator>;
        // static constexpr auto is_contiguous = is_vector_v<T> || is_string_v<T> || is_string_view_v<T>;



        struct Serializable
        {
            static constexpr staging_type stage(const value_type & value)
            {
                // collection size is limited by the type used to serialize it (`staging_type` aka uint32)
                // this can be changed to allow bigger collections, but it seems unlikely that would be used
                if constexpr (std::numeric_limits<staging_type>::max() < std::numeric_limits<std::size_t>::max())
                {
                    if (value.size() > std::numeric_limits<staging_type>::max())
                        throw std::length_error{"Serialization error: collection has too many elements"};
                }

                return staging_type(value.size());
            }

        };

        
        struct Deserializable
        {
            using staging_type = typename EIS2_Traits<T>::staging_type;

            static constexpr T stage_commit(staging_type) { return {}; }
        };
    };

}
