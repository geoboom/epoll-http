include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(BINARY server_test)

add_executable(
    ${BINARY}
    tests.cpp
    )

target_link_libraries(
    ${BINARY}
    PUBLIC
    gtest_main
    server_lib
    )

include(GoogleTest)
gtest_discover_tests(${BINARY})

