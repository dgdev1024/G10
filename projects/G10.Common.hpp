/**
 * @file    G10.Common.hpp
 * @brief   Contains includes and definitions commonly across all projects in
 *          the G10 Workspace.
 */

#pragma once

// Includes ********************************************************************

#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <deque>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <span>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>

#if defined(G10_CONFIG_DEBUG)
    #include <print>
    #define debug(...) std::println(__VA_ARGS__)
#else
    #define debug(...)
#endif

// Library Import/Export *******************************************************

#if defined(G10_BUILD_STATIC) || defined(G10_BUILD_APP)
    #define G10_API
#else
    #if defined(G10_SYSTEM_WINDOWS)
        #if defined(G10_BUILD_SHARED)
            #define G10_API __declspec(dllexport)
        #else
            #define G10_API __declspec(dllimport)
        #endif
    #else
        #define G10_API __attribute__((visibility("default")))
    #endif
#endif

// Macros - Strong-typed Bitfield Enums ****************************************

#define G10_BITFIELD_ENUM(ec, ...) \
    inline constexpr auto operator| (ec lhs, ec rhs) -> ec \
    { \
        return static_cast<ec>(std::to_underlying(lhs) | \
            std::to_underlying(rhs)); \
    } \
    inline constexpr auto operator& (ec lhs, ec rhs) -> ec \
    { \
        return static_cast<ec>(std::to_underlying(lhs) & \
            std::to_underlying(rhs)); \
    } \
    inline constexpr auto operator^ (ec lhs, ec rhs) -> ec \
    { \
        return static_cast<ec>(std::to_underlying(lhs) ^ \
            std::to_underlying(rhs)); \
    } \
    inline constexpr auto operator~ (ec lhs) -> ec \
    { \
        return static_cast<ec>(~std::to_underlying(lhs)); \
    } \
    inline constexpr auto operator|= (ec& lhs, ec rhs) -> void \
    { \
        lhs = lhs | rhs; \
    } \
    inline constexpr auto operator&= (ec& lhs, ec rhs) -> void \
    { \
        lhs = lhs & rhs; \
    } \
    inline constexpr auto operator^= (ec& lhs, ec rhs) -> void \
    { \
        lhs = lhs ^ rhs; \
    }

// Standard Library Extension **************************************************

namespace fs = std::filesystem;

namespace stx
{
    // *************************************************************************

    template <typename... Ts>
    struct overload : Ts... { using Ts::operator()...; };

    template <typename... Ts>
    overload(Ts...) -> overload<Ts...>;

    // *************************************************************************

    template <typename T>
    using expect = std::expected<T, std::string>;
    using expect_good = std::expected<void, std::string>;

    template <typename T>
    using ptr = T*;

    template <typename T>
    using dictionary = std::unordered_map<std::string, T>;

    template <typename T>
    using dual_vector = std::vector<std::vector<T>>;

    template <typename T>
    using sptr_vector = std::vector<std::shared_ptr<T>>;

    template <typename T, typename U>
    using optional_pair = std::optional<std::pair<T, U>>;

    template <typename... Ts>
    using optional_var = std::optional<std::variant<Ts...>>;

    template <typename T>
    auto clamp (T value, T lower, T upper) -> T;

    // *************************************************************************

    constexpr std::uint32_t npos32 = static_cast<std::uint32_t>(-1);
    constexpr std::uint64_t npos64 = static_cast<std::uint64_t>(-1);
    constexpr std::uint64_t npos = npos64;

    // *************************************************************************

    template <typename T>
        requires std::is_base_of_v<std::exception, T>
    class fmt_exception final : public T
    {
    public: // Constructors & Destructor ***************************************

        template <typename... As>
        inline explicit fmt_exception (std::string_view fmt, As... args) :
            T { std::vformat(fmt, std::make_format_args(args...)) } {}

    };

    using runtime_error = fmt_exception<std::runtime_error>;
    using invalid_argument = fmt_exception<std::invalid_argument>;
    using out_of_range = fmt_exception<std::out_of_range>;

    class byte_view final
    {
    public: // Constructors & Destructor ***************************************

        inline explicit byte_view (std::vector<std::uint8_t>& v) : view { v } {}

    public: // Methods *********************************************************

        inline auto reset () -> void
            { _read_ptr = 0; _write_ptr = 0; }
        inline auto reset (std::vector<std::uint8_t>& v) -> void
            { view = v; reset(); }

    public: // Methods - Pointers **********************************************

        inline auto read_ptr () const -> std::size_t
            { return _read_ptr; }
        inline auto write_ptr () const -> std::size_t
            { return _write_ptr; }
        inline auto set_read_ptr (std::size_t ptr) -> void
            { _read_ptr = clamp(ptr, std::size_t { 0 }, view.size()); }
        inline auto set_write_ptr (std::size_t ptr) -> void
            { _write_ptr = clamp(ptr, std::size_t { 0 }, view.size()); }
        inline auto sync_read_ptr () -> void
            { _read_ptr = clamp(_write_ptr, std::size_t { 0 }, view.size()); }
        inline auto sync_write_ptr () -> void
            { _write_ptr = clamp(_read_ptr, std::size_t { 0 }, view.size()); }

    public: // Methods - Read **************************************************

        inline auto read_byte () -> std::uint8_t
            { return view.at(_read_ptr++); }

        inline auto read_word_be () -> std::uint16_t
        { 
            return 
                (static_cast<std::uint16_t>(read_byte()) <<  8) | 
                (static_cast<std::uint16_t>(read_byte()) <<  0); 
        }

        inline auto read_word_le () -> std::uint16_t
        { 
            return 
                (static_cast<std::uint16_t>(read_byte()) <<  0) | 
                (static_cast<std::uint16_t>(read_byte()) <<  8); 
        }

        inline auto read_dword_be () -> std::uint32_t
        { 
            return 
                (static_cast<std::uint32_t>(read_byte()) << 24) | 
                (static_cast<std::uint32_t>(read_byte()) << 16) | 
                (static_cast<std::uint32_t>(read_byte()) <<  8) | 
                (static_cast<std::uint32_t>(read_byte()) <<  0); 
        }

        inline auto read_dword_le () -> std::uint32_t
        { 
            return 
                (static_cast<std::uint32_t>(read_byte()) <<  0) | 
                (static_cast<std::uint32_t>(read_byte()) <<  8) | 
                (static_cast<std::uint32_t>(read_byte()) << 16) | 
                (static_cast<std::uint32_t>(read_byte()) << 24); 
        }

        inline auto read_qword_le () -> std::uint64_t
        { 
            return 
                (static_cast<std::uint64_t>(read_byte()) <<  0) | 
                (static_cast<std::uint64_t>(read_byte()) <<  8) | 
                (static_cast<std::uint64_t>(read_byte()) << 16) | 
                (static_cast<std::uint64_t>(read_byte()) << 24) | 
                (static_cast<std::uint64_t>(read_byte()) << 32) | 
                (static_cast<std::uint64_t>(read_byte()) << 40) | 
                (static_cast<std::uint64_t>(read_byte()) << 48) | 
                (static_cast<std::uint64_t>(read_byte()) << 56); 
        }

        inline auto read_qword_be () -> std::uint64_t
        { 
            return 
                (static_cast<std::uint64_t>(read_byte()) << 56) | 
                (static_cast<std::uint64_t>(read_byte()) << 48) | 
                (static_cast<std::uint64_t>(read_byte()) << 40) | 
                (static_cast<std::uint64_t>(read_byte()) << 32) | 
                (static_cast<std::uint64_t>(read_byte()) << 24) | 
                (static_cast<std::uint64_t>(read_byte()) << 16) | 
                (static_cast<std::uint64_t>(read_byte()) <<  8) | 
                (static_cast<std::uint64_t>(read_byte()) <<  0); 
        }

        inline auto read_string () -> std::string
        {
            std::string str {};
            char c { '\0' };

            while (_read_ptr < view.size() && (c = read_byte()) != '\0')
            {
                str += c;
            }

            return str;
        }

        inline auto read_buffer (std::size_t size = stx::npos64)
            -> std::vector<std::uint8_t>
        {
            std::size_t remaining_buffer_size = view.size() - _read_ptr;
            size = std::min(size, remaining_buffer_size);
            
            std::vector<std::uint8_t> buffer {};
            buffer.insert(buffer.end(), view.begin() + _read_ptr, view.begin() + _read_ptr + size);
            _read_ptr += size;
            return buffer;
        }

        template <typename T>
        auto read () -> T;

    public: // Methods - Write *************************************************

        inline auto write_byte (std::uint8_t byte) -> void
            { view.at(_write_ptr++) = byte; }

        inline auto write_word_be (std::uint16_t word) -> void
        {
            write_byte(static_cast<std::uint8_t>((word >> 8) & 0xFF));
            write_byte(static_cast<std::uint8_t>((word >> 0) & 0xFF));
        }

        inline auto write_word_le (std::uint16_t word) -> void
        {
            write_byte(static_cast<std::uint8_t>((word >> 0) & 0xFF));
            write_byte(static_cast<std::uint8_t>((word >> 8) & 0xFF));
        }

        inline auto write_dword_be (std::uint32_t dword) -> void
        {
            write_byte(static_cast<std::uint8_t>((dword >> 24) & 0xFF));
            write_byte(static_cast<std::uint8_t>((dword >> 16) & 0xFF));
            write_byte(static_cast<std::uint8_t>((dword >> 8) & 0xFF));
            write_byte(static_cast<std::uint8_t>((dword >> 0) & 0xFF));
        }

        inline auto write_dword_le (std::uint32_t dword) -> void
        {
            write_byte(static_cast<std::uint8_t>((dword >> 0) & 0xFF));
            write_byte(static_cast<std::uint8_t>((dword >> 8) & 0xFF));
            write_byte(static_cast<std::uint8_t>((dword >> 16) & 0xFF));
            write_byte(static_cast<std::uint8_t>((dword >> 24) & 0xFF));
        }

        inline auto write_qword_be (std::uint64_t qword) -> void
        {
            write_byte(static_cast<std::uint8_t>((qword >> 56) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 48) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 40) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 32) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 24) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 16) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 8) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 0) & 0xFF));
        }

        inline auto write_qword_le (std::uint64_t qword) -> void
        {
            write_byte(static_cast<std::uint8_t>((qword >> 0) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 8) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 16) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 24) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 32) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 40) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 48) & 0xFF));
            write_byte(static_cast<std::uint8_t>((qword >> 56) & 0xFF));
        }

        inline auto write_string (const std::string& str) -> void
        {
            for (char c : str)
                { write_byte(static_cast<std::uint8_t>(c)); }

            write_byte(static_cast<std::uint8_t>('\0'));
        }

        inline auto write_buffer (const std::vector<std::uint8_t>& buffer) -> void
        {
            view.insert(view.begin() + _write_ptr, buffer.begin(), buffer.end());
            _write_ptr += buffer.size();
        }

        template <typename T>
        auto write (const T& value) -> void;

    public: // Methods - Push **************************************************

        inline auto push_byte (std::uint8_t byte) -> void
            { view.push_back(byte); }

        inline auto push_word_be (std::uint16_t word) -> void
        {
            push_byte(static_cast<std::uint8_t>((word >> 8) & 0xFF));
            push_byte(static_cast<std::uint8_t>((word >> 0) & 0xFF));
        }

        inline auto push_word_le (std::uint16_t word) -> void
        {
            push_byte(static_cast<std::uint8_t>((word >> 0) & 0xFF));
            push_byte(static_cast<std::uint8_t>((word >> 8) & 0xFF));
        }

        inline auto push_dword_be (std::uint32_t dword) -> void
        {
            push_byte(static_cast<std::uint8_t>((dword >> 24) & 0xFF));
            push_byte(static_cast<std::uint8_t>((dword >> 16) & 0xFF));
            push_byte(static_cast<std::uint8_t>((dword >> 8) & 0xFF));
            push_byte(static_cast<std::uint8_t>((dword >> 0) & 0xFF));
        }

        inline auto push_dword_le (std::uint32_t dword) -> void
        {
            push_byte(static_cast<std::uint8_t>((dword >> 0) & 0xFF));
            push_byte(static_cast<std::uint8_t>((dword >> 8) & 0xFF));
            push_byte(static_cast<std::uint8_t>((dword >> 16) & 0xFF));
            push_byte(static_cast<std::uint8_t>((dword >> 24) & 0xFF));
        }

        inline auto push_qword_be (std::uint64_t qword) -> void
        {
            push_byte(static_cast<std::uint8_t>((qword >> 56) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 48) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 40) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 32) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 24) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 16) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 8) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 0) & 0xFF));
        }

        inline auto push_qword_le (std::uint64_t qword) -> void
        {
            push_byte(static_cast<std::uint8_t>((qword >> 0) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 8) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 16) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 24) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 32) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 40) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 48) & 0xFF));
            push_byte(static_cast<std::uint8_t>((qword >> 56) & 0xFF));
        }

        inline auto push_string (const std::string& str) -> void
        {
            for (char c : str)
                { push_byte(static_cast<std::uint8_t>(c)); }

            push_byte(static_cast<std::uint8_t>('\0'));
        }

        inline auto push_buffer (const std::vector<std::uint8_t>& buffer) -> void
        {
            view.insert(view.end(), buffer.begin(), buffer.end());
        }

        template <typename T>
        auto push (const T& value) -> void;

    private: // Members ********************************************************

        std::vector<std::uint8_t>& view;  
        std::size_t _read_ptr { 0 };
        std::size_t _write_ptr { 0 };  

    };

    template <typename T>
    class optional_ref final
    {
    public: // Constructors & Destructor ***************************************

        inline optional_ref () = default;
        inline optional_ref (T& t) : ptr { &t } {}
        inline optional_ref (std::nullopt_t) : ptr { nullptr } {}
        inline optional_ref (std::nullptr_t) : ptr { nullptr } {}

    public: // Methods *********************************************************

        inline auto value_ptr () const -> T*
            { return ptr; }

        inline auto value () const -> T&
        {
            if (ptr == nullptr)
                { throw std::bad_optional_access {}; }

            return *ptr;
        }

        inline auto has_value () const -> bool
            { return (ptr != nullptr); }

        template <typename U>
        inline auto get_if () const -> U*
        {
            if constexpr (std::is_same_v<T, U> == true)
                { return ptr; }
            else
                { return nullptr; }
        }

        template <typename U>
        inline auto get (U def = U {}) -> U
        {
            if constexpr(std::is_same_v<T, U> == true)
            {
                if (ptr != nullptr)
                    { return *ptr; }
                else
                    { return def; }
            }
            else
                { return def; }
        }

        inline auto reset () -> void
            { ptr = nullptr; }

    public: // Operators *******************************************************

        inline auto operator= (T& t) -> optional_ref<T>&
            { ptr = &t; return *this; }
        inline auto operator-> () const -> T*
            { return value_ptr(); }
        inline auto operator* () const -> T&
            { return value(); }
        inline auto operator! () const -> bool
            { return (has_value() == false); }
        inline operator bool () const
            { return has_value(); }

    private: // Members ********************************************************

        T* ptr { nullptr };

    };

    // *************************************************************************

    template <typename T>
    inline auto clamp (T value, T lower, T upper) -> T
    {
        return (value < lower) ? lower :
            ((value > upper) ? upper : value);
    }

    template <typename T, typename U>
    inline auto to (std::shared_ptr<U>& ptr) -> std::shared_ptr<T>
        { return std::dynamic_pointer_cast<T>(ptr); }
    template <typename T, typename U>
    inline auto to (const std::shared_ptr<U>& ptr) -> std::shared_ptr<T>
        { return std::dynamic_pointer_cast<T>(ptr); }
    template <typename T>
        requires std::is_scoped_enum_v<T>
    inline auto under (const T& val) -> std::underlying_type_t<T>
        { return std::to_underlying(val); }

    template <typename... As>
    inline constexpr auto fmt_unexpected (std::string_view fmt, As&&... args)
        -> std::unexpected<std::string>
    {
        return std::unexpected<std::string> {
            std::vformat(
                fmt,
                std::make_format_args(args...)
            )
        };
    }

    inline auto load_binary (const fs::path& path, std::size_t offset = 0,
        std::size_t size = npos64) -> std::optional<std::vector<std::uint8_t>>
    {
        if (fs::exists(path) == false)
            { return std::nullopt; }

        // Get the file's size.
        auto file_size = fs::file_size(path);

        // Ensure the offset is valid (less than file size)
        if (offset >= file_size)
            { return std::nullopt; }

        // Determine the size to read
        if (size == npos64)
            { size = file_size - offset; }
        else if (size > file_size - offset)
            { size = file_size - offset; }

        // Read the binary data
        std::fstream file { path, std::ios::binary | std::ios::in };
        if (file.is_open() == false)
            { return std::nullopt; }

        file.seekg(offset, std::ios::beg);
        std::vector<std::uint8_t> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

}
