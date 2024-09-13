#pragma once

#include <type_traits>
#include <tuple>
#include <span>



namespace dd99::eis2
{

    // fw-declaration. Defined for (de)serializable types
    template <class T> struct EIS2_Traits;
    // remove references
    template <class T> struct EIS2_Traits<const T &> : EIS2_Traits<T> {};



    namespace internal
    {
        template <class> struct is_tuple : std::false_type {};
        template <class ...T> struct is_tuple<std::tuple<T...>>: std::true_type {};
        template <class T, class U> struct is_tuple<std::pair<T, U>>: std::true_type {};
        // template <class T, std::size_t N> struct is_tuple<std::array<T, N>>: std::true_type {};
        template <class T> inline constexpr bool is_tuple_v = is_tuple<T>::value;

        // tuple of removed-reference types
        // template <typename... T> using remove_ref_tuple = std::tuple<std::remove_reference_t<T>...>;
        template <class T> struct remove_ref_tuple;
        template <class ... T> struct remove_ref_tuple<std::tuple<T...>> : std::type_identity<std::tuple<std::remove_reference_t<T>...>> {};
        template <class T> using remove_ref_tuple_t = typename remove_ref_tuple<T>::type;

        template <class T> struct remove_cv_ref_tuple;
        template <class ... T> struct remove_cv_ref_tuple<std::tuple<T...>> : std::type_identity<std::tuple<std::remove_cvref_t<T>...>> {};
        template <class T> using remove_cv_ref_tuple_t = typename remove_cv_ref_tuple<T>::type;



        // can be memcopyed directly to output
        template <class T> concept Trivial = EIS2_Traits<std::remove_reference_t<T>>::is_trivial;

        template <class T> concept Collection = EIS2_Traits<std::remove_reference_t<T>>::is_collection;

        // elements are linear in memory with no padding between them (not counting internal padding)
        template <class T> concept Contiguous_Collection = Collection<T> && EIS2_Traits<std::remove_reference_t<T>>::is_contiguous;

        // collection where data() can be memcopyed directly to output
        template <class T> concept Trivial_Collection = Contiguous_Collection<T> && Trivial<typename std::remove_reference_t<T>::value_type>;



        template <class T> concept Symmetric_Aggregate = requires(const T & t) { EIS2_Traits<std::remove_reference_t<T>>::to_tuple(t); };


        

        template <class T> concept Serializable_Aggregate =
            Symmetric_Aggregate<T> ||
            requires(const T & t) { EIS2_Traits<T>::Serializable::to_tuple(t); };

        template <class T> concept Serializable_Transformable =
            requires(const T & t) { EIS2_Traits<T>::Serializable::transform(t); };

        template <class T> concept Serializable_Stageable =
            requires(const T & t) { EIS2_Traits<T>::Serializable::stage(t); };



        template <class T> concept Deserializable_Aggregate =
            Symmetric_Aggregate<T> ||
            requires (EIS2_Traits<T>::Deserializable::tuple_type tp)
            {
                // typename EIS2_Traits<T>::Deserializable::tuple_type;
                { EIS2_Traits<T>::Deserializable::from_tuple(std::move(tp)) } -> std::convertible_to<T>;
            };

        template <class T> concept Deserializable_Transformable =
            requires (EIS2_Traits<T>::Deserializable::transformed_type t)
            {
                // typename EIS2_Traits<T>::Deserializable::transformed_type;
                EIS2_Traits<T>::Deserializable::reverse_transform(std::move(t) );
            };

        template <class T> concept Deserializable_Stageable =
            requires (EIS2_Traits<T>::Deserializable::staging_type stg)
            {
                // typename EIS2_Traits<T>::Deserializable::staging_type;
                EIS2_Traits<T>::Deserializable::stage_commit(stg);
            };



        // // template <class T> concept Serializable_Tuple =
        // //     []<std::size_t ... N>(std::index_sequence<N...>)
        // //     { return Serializable<std::tuple_element_t<N, T>> && ...; }
        // //         (std::make_index_sequence<std::tuple_size_v<T>>());

        // // template <class T> concept Composite_Serializable = requires (const T & t) { { EIS2_Traits<T>::to_tuple(t) } -> Serializable_Tuple; };


        // template <class T> struct Serializable_Impl : std::false_type {};

        // // can be decomposed into tuple
        // template <class T> concept Decomposable = requires (const T & t) { EIS2_Traits<std::remove_cvref_t<T>>::Serializable::to_tuple(t); };

        // template <Trivially_Serializable T> struct Serializable_Impl<T> : std::true_type {};
        // template <Decomposable T> struct Serializable_Impl<T>
        //     : std::bool_constant<[]<std::size_t ... N>(std::index_sequence<N...>)
        //         {
        //             return std::conjunction_v<
        //                 Serializable_Impl<
        //                     std::tuple_element_t<N, decltype(EIS2_Traits<T>::Serializable::to_tuple(std::declval<const T &>()))>
        //                 >...
        //             >;
        //         }
        //         (std::make_index_sequence<std::tuple_size_v<decltype(EIS2_Traits<T>::Serializable::to_tuple(std::declval<const T &>()))>>())>
        // {};
        // template <class T> struct Serializable_Impl<T &> : Serializable_Impl<T> {};



        // template <class T> struct decompose_aggregate : std::type_identity<std::tuple<T>> {};
        // template <class T> using decompose_aggregate_t = typename decompose_aggregate<T>::type;

        // template <Deserializable_Aggregate T> struct decompose_aggregate<T>
        //     : std::type_identity<
        //         decompose_aggregate_t<
        //             typename EIS2_Traits<T>::Deserializable::tuple_type>>
        // { };

        // template <class ... Ts>
        // struct decompose_aggregate<std::tuple<Ts...>>
        //     : std::type_identity<
        //         decltype( std::tuple_cat(std::declval<decompose_aggregate_t<Ts>>() ...) )>
        // { };


        template <class T> concept Has_Serializable = requires { typename EIS2_Traits<T>::Serializable; };
        template <class T> concept Has_Deserializable = requires { typename EIS2_Traits<T>::Deserializable; };

    }


    
    // Deserializable concept: true if EIS2_Traits<T> defines nested type Serializable
    template <class T> concept Serializable =
        internal::Trivial<T> ||
        internal::Symmetric_Aggregate<T> ||
        internal::Has_Serializable<T>;



    // Deserializable concept: true if EIS2_Traits<T> defines nested type Deserializable
    template <class T> concept Deserializable =
        internal::Trivial<T> ||
        internal::Symmetric_Aggregate<T> ||
        internal::Has_Deserializable<T>;

}
