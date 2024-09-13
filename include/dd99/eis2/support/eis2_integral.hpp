#pragma once

#include <dd99/eis2/trivial.hpp>



namespace dd99::eis2
{

    // add some exceptions that can also be handled by the Serializable_Traits specialization for integrals
    template <class T>
    concept extended_integral =
        ::std::integral<T> ||
        ::std::same_as<::std::remove_cvref_t<T>, std::byte>;


    template <extended_integral T>
    struct EIS2_Traits<T>
    {
        using value_type = T;
        using transformed_type = trivial<value_type>;

        static constexpr auto serialized_endianness = std::endian::big;
        static constexpr auto is_trivial = (serialized_endianness == std::endian::native) || (sizeof(value_type) == 1);


        struct Serializable
        {
            static constexpr transformed_type transform(const value_type & value)
            requires (!is_trivial)
            { return {std::byteswap(value)}; }
        };


        struct Deserializable
        {
            using transformed_type = typename EIS2_Traits<T>::transformed_type;

            // static constexpr staging_type stage()
            // requires (!is_trivial)
            // { return staging_type{}; }

            static constexpr T reverse_transform(transformed_type && stage)
            requires (!is_trivial)
            { return std::byteswap(std::move(stage.value)); }
        };

    };


}
