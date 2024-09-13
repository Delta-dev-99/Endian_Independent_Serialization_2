#pragma once

#include <dd99/eis2/internal.hpp>
#include <dd99/eis2/io/output_device.hpp>



namespace dd99::eis2
{

    // fw-declaration
    template <Serializable T, dd99::eis2::io::OutputDevice OutputT>
    constexpr void serialize(OutputT & out, const T & value);


    namespace internal
    {

        template <class T>
        constexpr auto serializable_decompose(T && t)
        {
            using value_type = std::remove_cvref_t<T>;

            static_assert(
                Serializable_Aggregate<value_type>
                xor Serializable_Transformable<value_type>
                xor Serializable_Stageable<value_type>
                xor Trivial<value_type>
                xor is_tuple_v<value_type>);

            if constexpr (Symmetric_Aggregate<value_type>)
                return serializable_decompose(EIS2_Traits<value_type>::to_tuple(std::forward<T>(t)));
            else if constexpr (Serializable_Aggregate<value_type>)
                return serializable_decompose(EIS2_Traits<value_type>::Serializable::to_tuple(std::forward<T>(t)));
            else if constexpr (Serializable_Transformable<value_type>)
                return serializable_decompose(EIS2_Traits<value_type>::Serializable::transform(std::forward<T>(t)));
            else if constexpr (Serializable_Stageable<value_type> || Trivial<value_type>)
                // return std::forward_as_tuple(std::forward<T>(t));
                return std::tuple<T>{std::forward<T>(t)};
            else if constexpr (is_tuple_v<value_type>)
                return [&]<std::size_t ... N>(std::index_sequence<N...>){
                    return std::tuple_cat(serializable_decompose(std::get<N>(t))...);
                }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>());
                // return std::apply([](auto && ... ts)
                // {
                //     return std::tuple_cat(serializable_decompose(std::forward<decltype(ts)>(ts))...);
                // }, std::forward<T>(t));
        }


        // template <class T>
        // constexpr auto serializable_stage(T && t)
        // {
        //     using value_type = std::remove_cvref_t<T>;

        //     if constexpr (is_tuple_v<value_type>)
        //     {
        //         // return [&]<std::size_t ... N>(std::index_sequence<N...>){
        //         //     return std::tuple_cat(
        //         //         serializable_stage(
        //         //             std::forward<std::tuple_element_t<N, std::remove_reference_t<T>>>(
        //         //                 std::get<N>(t)))...);
        //         // }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>());
        //         return std::apply([](auto && ... ts)
        //         {
        //             return std::tuple_cat(serializable_stage(std::forward<decltype(ts)>(ts))...);
        //         }, std::forward<T>(t));
        //     }
        //     else if constexpr (Serializable_Stageable<value_type>)
        //         return std::make_tuple(EIS2_Traits<value_type>::Serializable::stage(std::forward<T>(t)));
        //     else
        //         return std::forward_as_tuple(std::forward<T>(t));
        // }

        template <class T>
        constexpr decltype(auto) serializable_stage(T && t)
        {
            using value_type = std::remove_cvref_t<T>;

            if constexpr (is_tuple_v<value_type>)
            {
                // return [&]<std::size_t ... N>(std::index_sequence<N...>){
                //     return std::tuple_cat(
                //         serializable_stage(
                //             std::forward<std::tuple_element_t<N, std::remove_reference_t<T>>>(
                //                 std::get<N>(t)))...);
                // }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>());
                return std::apply([](auto && ... ts){
                    return std::tuple<decltype(serializable_stage(std::forward<decltype(ts)>(ts)))...>(
                        serializable_stage(std::forward<decltype(ts)>(ts))...);
                }, std::forward<T>(t));
            }
            else if constexpr (Serializable_Stageable<value_type>)
                return EIS2_Traits<value_type>::Serializable::stage(std::forward<T>(t));
            else
                return std::forward<T>(t);
        }


        template <class T>
        constexpr auto serializable_buffer(T & t)
        {   
            using value_type = std::remove_cvref_t<T>;

            if constexpr (Trivial<value_type>)
                return std::make_tuple(std::span{reinterpret_cast<const std::byte *>(std::addressof(t)), sizeof(t)});
            else if constexpr (is_tuple_v<value_type>)
                return std::apply([](auto & ... ts){
                    return std::tuple_cat(serializable_buffer(ts)...);
                }, t);
            else
                return EIS2_Traits<value_type>::Serializable::buffer(t);
        }


        template <class T, dd99::eis2::io::OutputDevice OutputT>
        constexpr auto serialize_collection_data(OutputT & out, T && t)
        {
            using value_type = std::remove_cvref_t<T>;

            if constexpr (Collection<value_type>)
            {
                if constexpr (Trivial_Collection<value_type>)
                    dd99::eis2::io::write(out, serializable_collection_data_buffer(t));
                else
                    for (const auto & x : collection_as_range(t))
                        dd99::eis2::serialize(out, x);
            }
            else if constexpr (is_tuple_v<value_type>)
                std::apply([&](auto && ... ts){
                    (serialize_collection_data(out, std::forward<decltype(ts)>(ts)), ...);
                }, std::forward<T>(t));
        }

        // template <class T, dd99::eis2::io::OutputDevice OutputT>
        // constexpr auto serializable_stage_commit(OutputT & out, T && t)
        // {
        //     using value_type = std::remove_cvref_t<T>;
            
        //     if constexpr (Serializable_Stageable<value_type>)
        //     {
        //         if constexpr ( requires { EIS2_Traits<value_type>::Serializable::stage_commit(std::forward<T>(t)); } )
        //             dd99::eis2::io::write(out, EIS2_Traits<value_type>::Serializable::stage_commit(std::forward<T>(t)));
        //         else if constexpr ( requires { EIS2_Traits<value_type>::Serializable::stage_commit(out, std::forward<T>(t)); } )
        //             EIS2_Traits<value_type>::Serializable::stage_commit(out, std::forward<T>(t));
        //     }
        //     else if constexpr (is_tuple_v<value_type>)
        //         std::apply([&](auto && ... ts){
        //             (serializable_stage_commit(out, std::forward<decltype(ts)>(ts)), ...);
        //         }, std::forward<T>(t));

        // }

    }


    template <Serializable T, dd99::eis2::io::OutputDevice OutputT>
    constexpr void serialize(OutputT & out, const T & value)
    {
        // decompose data
        auto decomposed = dd99::eis2::internal::serializable_decompose(value);

        // create staging storage when needed
        auto stage = dd99::eis2::internal::serializable_stage(decomposed);
        
        // decompose staging types
        auto stage_decomposed = dd99::eis2::internal::serializable_decompose(stage);

        // create buffers
        auto buffers = dd99::eis2::internal::serializable_buffer(stage_decomposed);

        // write data
        dd99::eis2::io::write(out, buffers);

        dd99::eis2::internal::serialize_collection_data(out, decomposed);

        // post-stage commit for extra (usually dynamic) data
        // dd99::eis2::internal::serializable_stage_commit(out, decomposed);

    };

}
