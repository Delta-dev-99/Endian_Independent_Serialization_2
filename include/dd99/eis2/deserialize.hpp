#pragma once

#include <dd99/eis2/internal.hpp>
#include <dd99/eis2/io/input_device.hpp>



namespace dd99::eis2
{

    // fw-declaration
    template <Deserializable T, class InputT> constexpr auto deserialize(InputT && in) -> T;


    namespace internal
    {

        // gets first stage storage where we put read data
        template <class T> constexpr auto deserializable_decompose()
        {
            if (!std::is_constant_evaluated()) throw std::logic_error{"Use in unevaluated context only!"};

            // assert (only) one option is valid
            static_assert(
                Deserializable_Aggregate<T>
                xor Deserializable_Transformable<T>
                xor Deserializable_Stageable<T>
                xor Trivial<T>
                xor is_tuple_v<T>);

            if constexpr (Symmetric_Aggregate<T>)
                return deserializable_decompose<remove_cv_ref_tuple_t<decltype(EIS2_Traits<T>::to_tuple(std::declval<T &>()))>>();
            else if constexpr (Deserializable_Aggregate<T>)
                return deserializable_decompose<typename EIS2_Traits<T>::Deserializable::tuple_type>();
            else if constexpr (Deserializable_Transformable<T>)
                return deserializable_decompose<typename EIS2_Traits<T>::Deserializable::transformed_type>();
            else if constexpr (Deserializable_Stageable<T> || Trivial<T>)
                return std::make_tuple(T{});
            else if constexpr (is_tuple_v<T>)
                return []<std::size_t ... N>(std::index_sequence<N...>){
                    return std::tuple_cat(deserializable_decompose<std::tuple_element_t<N, T>>()...);
                }(std::make_index_sequence<std::tuple_size_v<T>>());
        }

        template <class T> constexpr auto deserializable_stage()
        {
            if (!std::is_constant_evaluated()) throw std::logic_error{"Use in unevaluated context only!"};

            if constexpr (Deserializable_Stageable<T>)
                return typename EIS2_Traits<T>::Deserializable::staging_type{};
            else if constexpr (is_tuple_v<T>)
                return []<std::size_t ... N>(std::index_sequence<N...>){
                    return std::make_tuple(deserializable_stage<std::tuple_element_t<N, T>>()...);
                }(std::make_index_sequence<std::tuple_size_v<T>>());
            else
                return T{};
        }

        template <class T> constexpr auto deserializable_buffer(T & value)
        {   
            using value_type = std::remove_cvref_t<T>;

            if constexpr (Trivial<value_type>)
                return std::make_tuple(std::span{reinterpret_cast<std::byte *>(std::addressof(value)), sizeof(value)});
            else if constexpr (is_tuple_v<value_type>)
                return std::apply([](auto & ... xs){
                    return std::tuple_cat(deserializable_buffer(xs)...);
                }, value);
            else 
                return EIS2_Traits<value_type>::Deserializable::buffer(value);
        }


        template <std::size_t N, std::size_t Skip = 0, class T>
        constexpr auto tuple_take_n(T && t)
        {
            return [&]<std::size_t ...I>(std::index_sequence<I...>) {
                return std::make_tuple(std::get<I + Skip>(std::forward<T>(t))...);
            }( std::make_index_sequence<N>{} );
        }

        template <class T> constexpr T deserializable_compose(decltype(deserializable_decompose<T>()) && value)
        {
            if constexpr (Symmetric_Aggregate<T>)
                return std::make_from_tuple<T>(deserializable_compose<remove_cv_ref_tuple_t<decltype(EIS2_Traits<T>::to_tuple(std::declval<T &>()))>>(std::move(value)));
            else if constexpr (Deserializable_Aggregate<T>)
                return EIS2_Traits<T>::Deserializable::from_tuple(deserializable_compose<typename EIS2_Traits<T>::Deserializable::tuple_type>(std::move(value)));
            else if constexpr (Deserializable_Transformable<T>)
                return EIS2_Traits<T>::Deserializable::reverse_transform(deserializable_compose<typename EIS2_Traits<T>::Deserializable::transformed_type>(std::move(value)));
            else if constexpr (Deserializable_Stageable<T> || Trivial<T>)
                return std::make_from_tuple<T>(std::move(value));
            else if constexpr (is_tuple_v<T>)
                return [/* value = std::forward<decltype(value)>(value) */ &]<std::size_t ... N>(std::index_sequence<N...>){
                    return std::make_tuple(
                        deserializable_compose<std::tuple_element_t<N, T>>(
                            tuple_take_n<
                                std::tuple_size_v<decltype(deserializable_decompose<std::tuple_element_t<N, T>>())>,
                                []<std::size_t ... NN>(std::index_sequence<NN...>){
                                    return (std::tuple_size_v<decltype(deserializable_decompose<std::tuple_element_t<NN, T>>())> + ... + 0);
                                }(std::make_index_sequence<N>())
                            >(value)
                        )...
                    );
                }(std::make_index_sequence<std::tuple_size_v<T>>());
        }

        template <class T> constexpr T deserializable_stage_commit(decltype(deserializable_stage<T>()) & value)
        {
            if constexpr (Deserializable_Stageable<T>)
                return EIS2_Traits<T>::Deserializable::stage_commit(value);
            else if constexpr (is_tuple_v<T>)
                return [&]<std::size_t ... N>(std::index_sequence<N...>) {
                    return std::make_tuple(deserializable_stage_commit<std::tuple_element_t<N, T>>(std::get<N>(value))...);
                }(std::make_index_sequence<std::tuple_size_v<T>>());
            else return value;
        }


        template <class T, std::convertible_to<typename T::value_type> Elem> constexpr void deserializable_collection_insert(T & t, Elem && elem)
        {
            if constexpr ( requires { EIS2_Traits<T>::Deserializable::insert(t, std::move(elem)); } )
                return EIS2_Traits<T>::Deserializable::insert(t, std::move(elem));
            else
            {
                constexpr auto is_unordered_associative = requires { typename T::key_equal; };
                constexpr auto is_ordered_associative = requires { typename T::key_compare; };
                constexpr auto can_emplace_back = requires { t.emplace_back(std::move(elem)); };

                if constexpr (is_unordered_associative)
                    t.emplace(std::move(elem));
                else if constexpr (is_ordered_associative)
                    t.emplace_hint(t.end(), std::move(elem));
                else if constexpr (can_emplace_back)
                    t.emplace_back(std::move(elem));
                else // just in case, fallback
                    t.emplace(std::move(elem));
            }
        }

        
        template <class T, dd99::eis2::io::InputDevice InputT>
        constexpr void deserialize_collection_data(InputT & input, T & data, const decltype(dd99::eis2::internal::deserializable_stage<T>()) & stage)
        {
            if constexpr (Collection<T>)
            {
                if constexpr (Trivial_Collection<T>)
                {
                    data.resize(stage);
                    dd99::eis2::io::read(input, serializable_collection_data_buffer(data));
                }
                else
                {
                    for (auto c = stage; c > 0; --c)
                        deserializable_collection_insert(data,
                            deserialize<typename std::remove_cvref_t<T>::value_type>(input));
                }
            }
            else if constexpr (is_tuple_v<T>)
                std::apply([&](auto && ... datas){
                    std::apply([&](auto && ... stages){
                        (deserialize_collection_data(input, datas, stages), ...);
                    }, stage);
                }, data);
        }


        // template <class T>
        // constexpr T deserializable_stage_commit(decltype(deserializable_stage<T>()) && stage)
        // {
        //     if constexpr (!Deserializable_Stageable<T>)
        //     {
        //         // allow pass-through for non-stageable types
        //         static_assert(std::same_as<T, decltype(stage)>);
        //         return std::move(stage);
        //     }
        //     else
        //     {
        //         return EIS2_Traits<T>::Deserializable::stage_commit(
        //             deserializable_stage_commit<typename EIS2_Traits<T>::staging_type>(std::move(stage))
        //         );
        //     }
        // }

        // template <class ... T>
        // constexpr auto deserializable_stage_commit<std::tuple<T...>>(decltype(deserializable_stage<std::tuple<T...>>()) stage)
        // {
        //     return std::apply([](auto && ... stage_elems)
        //     {
        //         return std::make_tuple(deserializable_stage_commit<T>(std::move(stage_elems))...);
        //     }, std::move(stage)); 
        // }

    }


    template <Deserializable T, class InputT>
    constexpr auto deserialize(InputT && in)
    -> T
    {
        using dd99::eis2::io::read;

        using decomposed_t = decltype(dd99::eis2::internal::deserializable_decompose<T>());

        using stage_t = decltype(dd99::eis2::internal::deserializable_stage<decomposed_t>());

        using stage_decomposed_t = decltype(dd99::eis2::internal::deserializable_decompose<stage_t>());

        stage_decomposed_t stage_decomposed;

        auto buffers = dd99::eis2::internal::deserializable_buffer(stage_decomposed);

        read(in, buffers);

        auto stage = internal::deserializable_compose<stage_t>(std::move(stage_decomposed));

        auto decomposed = internal::deserializable_stage_commit<decomposed_t>(stage);

        internal::deserialize_collection_data(in, decomposed, stage);

        return internal::deserializable_compose<T>(std::move(decomposed));
    }

}
