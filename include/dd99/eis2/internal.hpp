#pragma once

#include <dd99/eis2/eis2_traits.hpp>
#include <functional>



namespace dd99::eis2::internal
{

    
    // template <class T> consteval bool is_collection()
    // {
    //     if constexpr( requires { EIS2_Traits<T>::is_collection; } )
    //         return EIS2_Traits<T>::is_collection;
    //     else return false;
    // }

    // template <class T> consteval bool is_trivially_serializable()
    // {
    //     if constexpr( requires { EIS2_Traits<T>::is_trivially_serializable; } )
    //         return EIS2_Traits<T>::is_trivially_serializable;
    //     else return false;
    // }

    // template <class T> consteval bool is_contiguous_collection()
    // {
    //     if constexpr( requires { EIS2_Traits<T>::is_collection; EIS2_Traits<T>::is_contiguous; } )
    //         return EIS2_Traits<T>::is_collection && EIS2_Traits<T>::is_contiguous; 
    //     else return false;
    // }


    // template <class T> consteval bool is_trivial_collection()
    // {
    //     if constexpr (!Contiguous_Collection<T>) return false;
    //     else return Trivially_Serializable<typename T::value_type>;
    // }

    // template <Trivial_Collection T> constexpr auto collection_data_buffer(const T & v)
    // {
    //     return std::span<const std::byte>{
    //         reinterpret_cast<const std::byte *>(v.data()),
    //         v.size() * sizeof(typename T::value_type)};
    // }


    // <Serialization by composition>
    // This function "unpacks" types that can be serialized by treating them as tuples of member elements
    // value is either a single element or a tuple
    // returns tuple of unpacked serializable elements
    // returned elements can't be further decomposed into serializable elements
    template <class T>
    constexpr auto serializable_full_unpack(T & value)
    {
        using value_type = std::remove_cvref_t<T>;
        using traits = EIS2_Traits<value_type>;

        if constexpr ( requires { traits::Serializable::to_tuple(value); } )
        {
            return std::apply([](auto & ... x){
                return std::tuple_cat(serializable_full_unpack(x)...);
            }, traits::Serializable::to_tuple(value));
        }
        else if constexpr ( is_tuple_v<value_type> )
        {
            return std::apply([](auto & ... x){
                return std::tuple_cat(serializable_full_unpack(x)...);
            }, value);
        }
        else return std::tie(value);
    }

    template <Trivial_Collection T>
    constexpr auto serializable_collection_data_buffer(const T & value)
    {
        using value_type = std::remove_cvref_t<T>;
        using traits = EIS2_Traits<value_type>;
        using element_type = typename value_type::value_type;

        if constexpr ( requires { traits::Serializable::data_buffer(value); } )
            return traits::Serializable::data_buffer(value);
        else
        {
            return std::span<const std::byte>{
                reinterpret_cast<const std::byte *>(value.data()),
                value.size() * sizeof(element_type)};
        }
    }



    template <Collection T>
    constexpr auto collection_as_range(const T & value)
    -> decltype(EIS2_Traits<std::remove_cvref_t<T>>::Serializable::collection_as_range(value))
    requires requires { EIS2_Traits<std::remove_cvref_t<T>>::Serializable::collection_as_range(value); }
    {
        using value_type = std::remove_cvref_t<T>;
        using traits = EIS2_Traits<value_type>;

        if constexpr ( requires { traits::Serializable::collection_as_range(value); } )
            return traits::Serializable::collection_as_range(value);
        else return value;
    }

    template <Collection T>
    constexpr auto collection_as_range(const T & value)
    -> const T &
    requires requires { value.begin(); value.end(); }
    {
        return value;
    }


}
