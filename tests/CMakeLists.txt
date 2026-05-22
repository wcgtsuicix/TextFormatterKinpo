include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Не переопределять флаги CRT у родительского проекта на Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
        tests.cpp
)

target_link_libraries(unit_tests
        GTest::gtest
        GTest::gtest_main
        core
)

include(GoogleTest)
gtest_discover_tests(unit_tests
        DISCOVERY_MODE POST_BUILD)