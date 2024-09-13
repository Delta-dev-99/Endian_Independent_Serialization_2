#include <dd99/eis2/eis2.hpp>
#include <dd99/eis2/io/support/output_to_container.hpp>
#include <dd99/eis2/io/support/input_from_iterator.hpp>
#include <vector>
#include <list>
#include <coroutine>
#include <optional>
#include <cassert>


struct A
{
    int x, y;
    // std::array<char, 10> c;

    friend constexpr bool operator == (const A & a, const A & b)
    {
        return (a.x == b.x)
            && (a.y == b.y);
    }
};

struct B
{
    std::vector<A> v;

    friend constexpr bool operator == (const B & a, const B & b)
    { return a.v == b.v; }
};

struct C
{
    B b1, b2;
    std::optional<int> o;
    char c;

    friend constexpr bool operator == (const C &, const C &) = default;
    // friend constexpr bool operator == (const C & a, const C & b)
    // {
    //     return (a.b1 == b.b1)
    //         && (a.b2 == b.b2)
    //         && (a.o == b.o)
    //         && (a.c == b.c);
    // }
};


template <>
struct dd99::eis2::EIS2_Traits<A>
{
    using value_type = A;

    struct Serializable
    {
        static constexpr auto to_tuple(const value_type & value)
        // { return std::tie(value.x, value.c); }
        { return std::tie(value.x, value.y); }
    };

    struct Deserializable
    {
        // using tuple_type = std::tuple<int, std::array<char, 10>>;
        using tuple_type = std::tuple<int, int>;

        static constexpr value_type from_tuple(tuple_type && tp)
        { return std::make_from_tuple<value_type>(std::move(tp)); }
    };
};

template <>
struct dd99::eis2::EIS2_Traits<B>
{
    using value_type = B;

    struct Serializable
    {
        static constexpr auto to_tuple(const value_type & value)
        { return std::tie(value.v); }
    };

    struct Deserializable
    {
        using tuple_type = std::tuple<std::vector<A>>;

        static constexpr value_type from_tuple(tuple_type && tp)
        { return std::make_from_tuple<value_type>(std::move(tp)); }
    };
};

template <>
struct dd99::eis2::EIS2_Traits<C>
{
    using value_type = C;

    static constexpr auto to_tuple(const value_type & value)
    { return std::tie(value.b1, value.b2, value.o, value.c); }

    // struct Serializable
    // {
    //     static constexpr auto to_tuple(const value_type & value)
    //     { return std::tie(value.b1, value.b2, value.o); }
    // };

    // struct Deserializable
    // {
    //     using tuple_type = std::tuple<B, B, std::optional<int>>;

    //     static constexpr value_type from_tuple(tuple_type && tp)
    //     { return std::make_from_tuple<value_type>(std::move(tp)); }
    // };
};




// struct vector_output
// {
//     std::vector<std::byte> data;

//     template <dd99::eis2::io::ConstBufferSequence T>
//     // requires requires(T & t){ t.begin(); t.end(); {*t.begin()} -> std::convertible_to<const std::span<const std::byte>>; }
//     void write(const T & buffers)
//     {
//         std::size_t size = 0;
//         for (const auto & x : buffers)
//             size += x.size();

//         data.reserve(data.size() + size);

//         for (const auto & x : buffers)
//             std::copy(x.begin(), x.end(), std::back_inserter(data));
//     }

//     template <dd99::eis2::io::ConstBuffer T>
//     // template <std::convertible_to<const std::span<const std::byte>> T>
//     void write(const T & buffer)
//     {
//         data.resize(data.size() + buffer.size());
//         std::copy(buffer.begin(), buffer.end(), data.end() - buffer.size());
//     }
// };


template <class T>
void test(const T & t)
{
    dd99::eis2::io::Output_To_Container<std::vector<std::byte>> out;
    dd99::eis2::serialize(out, t);
    auto u = dd99::eis2::deserialize<T>(dd99::eis2::io::Input_From_Iterator{out.m_container.begin()});
    assert(t == u);
}




void f()
{
    // std::optional<int> opt;

    // using T = std::vector<A>;

    // using TT = dd99::eis2::EIS2_Traits<T>;

    // constexpr auto t1 = dd99::eis2::internal::Serializable_Aggregate<T>;
    // constexpr auto t2 = dd99::eis2::internal::Serializable_Transformable<T>;
    // constexpr auto t3 = dd99::eis2::internal::Deserializable_Stageable<T>;
    // constexpr auto t4 = dd99::eis2::internal::Trivial<T>;
    // constexpr auto t5 = dd99::eis2::internal::is_tuple_v<T>;


    // constexpr auto b1 = dd99::eis2::internal::Serializable_Aggregate<B>;
    // constexpr auto b2 = dd99::eis2::internal::Serializable_Transformable<B>;
    // constexpr auto b3 = dd99::eis2::internal::Trivial<B>;
    // constexpr auto b4 = dd99::eis2::internal::is_tuple_v<B>;


    // int x = 5;
    // std::span<std::byte, sizeof(x)> buf{reinterpret_cast<std::byte *>(std::addressof(x)), sizeof(x)};

    // std::vector<std::byte> data;
    // std::copy(buf.begin(), buf.end(), std::back_inserter(data));

    dd99::eis2::io::Output_To_Container<std::vector<std::byte>> o;
    // o.write(std::initializer_list<const std::span<const std::byte>>{buf, buf});

    C value{};
    value.b1.v.emplace_back();
    value.b1.v[0].x = 654;
    value.b1.v[0].y = 123;
    dd99::eis2::serialize(o, value);


    {
        using type_t = C;
        using decomposed_t = decltype(dd99::eis2::internal::serializable_decompose(std::declval<const type_t &&>()));
        using stage_t = decltype(dd99::eis2::internal::serializable_stage(std::declval<const decomposed_t &>()));
        using stage_decomposed_t [[maybe_unused]] = decltype(dd99::eis2::internal::serializable_decompose(std::declval<const stage_t &>()));

        auto decomposed = dd99::eis2::internal::serializable_decompose(value);
        auto stage = dd99::eis2::internal::serializable_stage(decomposed);
        auto stage_decomposed = dd99::eis2::internal::serializable_decompose(stage);
        auto buffers [[maybe_unused]] = dd99::eis2::internal::serializable_buffer(stage_decomposed);
    }


    {
        // char v;
        // std::tuple<char> v;
        std::tuple<std::tuple<char>> v;

        using decomposed_t = decltype(dd99::eis2::internal::serializable_stage(v));

        auto decomposed1 = dd99::eis2::internal::serializable_stage(v);
    }


    {
        using type_t = C;
        using decomposed_t = decltype(dd99::eis2::internal::deserializable_decompose<type_t>());
        using stage_t = decltype(dd99::eis2::internal::deserializable_stage<decomposed_t>());
        using stage_decomposed_t = decltype(dd99::eis2::internal::deserializable_decompose<stage_t>());
        stage_decomposed_t stage_decomposed;
        auto buffers [[maybe_unused]] = dd99::eis2::internal::deserializable_buffer(stage_decomposed);
    }

    // A a;
    // auto decomposed = dd99::eis2::internal::serializable_decompose(a);

    // int x;
    // auto decomposedx = dd99::eis2::internal::serializable_decompose(x);
    // dd99::eis2::trivial<int> y;
    // static_assert(dd99::eis2::internal::Trivial<decltype(y)>);
    // y.value = 15;
    // auto decomposedy = 
    //     dd99::eis2::internal::serializable_decompose(y);

    // // create staging storage for data that requires transformation
    // auto stage = std::apply([](const auto & ... x){
    //     return std::forward_as_tuple(dd99::eis2::internal::serializable_stage(x) ...);
    // }, decomposed);



    // constexpr auto b1 = dd99::eis2::EIS2_Traits<unsigned int>::is_trivially_serializable;
    // constexpr auto b2 = dd99::eis2::Trivially_Serializable<unsigned int>;

    // static_assert(dd99::eis2::internal::Trivial<const int &>);

    // using K2 = dd99::eis2::internal::Serializable_Impl<
    

    // auto vec = o.data;
    // dd99::eis2::serialize(vec, o);

    // auto unpacked = dd99::eis2::internal::serializable_full_unpack(value);

    // auto stage = std::apply([](const auto & ... x){
    //     return std::make_tuple(dd99::eis2::internal::serializable_stage(x) ...);
    // }, unpacked);


    // auto buffers = std::apply([](const auto & ... x){
    //     return std::make_tuple(dd99::eis2::internal::serializable_buffer(x) ...);
    // }, stage);





    dd99::eis2::io::Input_From_Iterator input{o.m_container.begin()};
    auto value_deserialized = dd99::eis2::deserialize<decltype(value)>(input);


    bool assert_b = value == value_deserialized;
    assert(assert_b);
}


int main()
{

    test(1<<0); test(1<<1); test(1<<2); test(1<<3);
    test(1<<4); test(1<<5); test(1<<6); test(1<<7);
    test(1<<8); test(1<<9); test(1<<10); test(1<<11);
    test(1<<12); test(1<<13); test(1<<14); test(1<<15);
    test(1<<16); test(1<<17); test(1<<18); test(1<<19);
    test(1<<20); test(1<<21); test(1<<22); test(1<<23);
    test(1<<24); test(1<<25); test(1<<26); test(1<<27);
    test(1<<28); test(1<<29); test(1<<30); test(1<<31);

    auto test64 = test<std::uint64_t>;

    test64(1<<0); test64(1<<1); test64(1<<2); test64(1<<3);
    test64(1<<28); test64(1<<29); test64(1<<30); test64(1<<31);
    test64(1ull<<47); test64(1ull<<48); test64(1ull<<49); test64(1ull<<50);

    test(std::vector<int>{1,5,6,7,10});

    
    // f();

}
