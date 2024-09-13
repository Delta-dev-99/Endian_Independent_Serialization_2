#pragma once

#include <dd99/eis2/trivial.hpp>
#include <dd99/eis2/eis2_traits.hpp>



namespace dd99::eis2
{

    template <class T>
    struct EIS2_Traits<dd99::eis2::trivial<T>>
    {
        static_assert(sizeof(T) == sizeof(dd99::eis2::trivial<T>));

        static constexpr bool is_trivial = true;
    };

}
