#pragma once

#include <dd99/eis2/eis2_traits.hpp>
// #include <dd99/eis2/internal.hpp>
#include <optional>
#include <cstdint>



namespace dd99::eis2
{

    template <Serializable T>
    struct EIS2_Traits<std::optional<T>>
    {
        using value_type = std::optional<T>;
        using staging_type = std::uint8_t;

        static constexpr auto is_collection = true;
        // static constexpr auto is_contiguous = true;


        struct Serializable
        {
            static constexpr staging_type stage(const value_type & value)
            { return staging_type{value.has_value()}; }

            static constexpr std::span<const T> collection_as_range(const value_type & v)
            {
                if (v.has_value()) return { std::addressof(*v), 1 };
                else return {};
            }
        };

        struct Deserializable
        {
            using staging_type = typename EIS2_Traits<std::optional<T>>::staging_type;

            static constexpr std::optional<T> stage_commit(staging_type) { return {}; }
        };
    };

}
