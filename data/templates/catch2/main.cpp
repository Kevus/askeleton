#define CATCH_CONFIG_MAIN
#if __has_include(<catch2/catch_all.hpp>)
#include <catch2/catch_all.hpp>
#elif __has_include(<catch2/catch.hpp>)
#include <catch2/catch.hpp>
#else
#error "Catch2 headers not found"
#endif
