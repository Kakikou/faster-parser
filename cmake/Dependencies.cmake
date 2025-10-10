include(FetchContent)

# Set FetchContent properties for better performance
#set(FETCHCONTENT_QUIET ON)
#set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

function(fetch_dependency name git_repo git_tag)
    message(STATUS "Fetching ${name} from GitHub...")
    FetchContent_Declare(
        ${name}
        GIT_REPOSITORY ${git_repo}
        GIT_TAG        ${git_tag}
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(${name})
endfunction()

if(BUILD_TESTS AND NOT TARGET gtest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GTEST ON CACHE BOOL "" FORCE)

    fetch_dependency(
        googletest
        https://github.com/google/googletest.git
        v1.17.0
    )

    if(TARGET gtest AND NOT TARGET GTest::gtest)
        add_library(GTest::gtest ALIAS gtest)
    endif()
    if(TARGET gtest_main AND NOT TARGET GTest::gtest_main)
        add_library(GTest::gtest_main ALIAS gtest_main)
    endif()
    if(TARGET gmock AND NOT TARGET GTest::gmock)
        add_library(GTest::gmock ALIAS gmock)
    endif()
    if(TARGET gmock_main AND NOT TARGET GTest::gmock_main)
        add_library(GTest::gmock_main ALIAS gmock_main)
    endif()
endif()

if(BUILD_BENCHMARKS AND NOT TARGET benchmark::benchmark)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)

    fetch_dependency(
        benchmark
        https://github.com/google/benchmark.git
        v1.9.4
    )

    # Fetch simdjson for benchmark comparisons only
    set(SIMDJSON_JUST_LIBRARY ON CACHE BOOL "" FORCE)
    set(SIMDJSON_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(SIMDJSON_ENABLE_THREADS ON CACHE BOOL "" FORCE)
    set(SIMDJSON_DEVELOPMENT_CHECKS OFF CACHE BOOL "" FORCE)

    fetch_dependency(
        simdjson
        https://github.com/simdjson/simdjson.git
        v3.11.4
    )
endif()

if(BUILD_TESTS OR BUILD_BENCHMARKS)
    message(STATUS "Dependencies summary:")
    if(BUILD_TESTS)
        message(STATUS "  - GoogleTest v1.17.0 (testing)")
    endif()
    if(BUILD_BENCHMARKS)
        message(STATUS "  - Google Benchmark v1.9.4 (benchmarking)")
        message(STATUS "  - simdjson v3.11.4 (benchmarking comparison)")
    endif()
endif()