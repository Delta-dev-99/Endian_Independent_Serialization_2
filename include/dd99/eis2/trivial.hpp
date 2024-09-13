#pragma once

#include <type_traits>



namespace dd99::eis2
{

    template <class T>
    struct trivial
    {
        std::remove_const_t<T> value;
    };

}
