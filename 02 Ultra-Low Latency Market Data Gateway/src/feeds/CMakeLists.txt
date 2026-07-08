include(CheckCXXCompilerFlag)

set(ULL_FEEDS_SOURCES
    feed_handler.cpp
    structural_scan.cpp
    structural_scan_scalar.cpp
    data_access_view.cpp
)

if(MSVC)
    check_cxx_compiler_flag("/arch:AVX2" ULL_COMPILER_HAS_AVX2)
    check_cxx_compiler_flag("/arch:AVX512" ULL_COMPILER_HAS_AVX512)
else()
    check_cxx_compiler_flag("-mavx2" ULL_COMPILER_HAS_AVX2)
    check_cxx_compiler_flag("-mavx512f" ULL_COMPILER_HAS_AVX512F)
    check_cxx_compiler_flag("-mavx512bw" ULL_COMPILER_HAS_AVX512BW)
    if(ULL_COMPILER_HAS_AVX512F AND ULL_COMPILER_HAS_AVX512BW)
        set(ULL_COMPILER_HAS_AVX512 TRUE)
    else()
        set(ULL_COMPILER_HAS_AVX512 FALSE)
    endif()
endif()

if(ULL_COMPILER_HAS_AVX2)
    list(APPEND ULL_FEEDS_SOURCES structural_scan_avx2.cpp)
endif()

if(ULL_COMPILER_HAS_AVX512)
    list(APPEND ULL_FEEDS_SOURCES structural_scan_avx512.cpp)
endif()

add_library(ull_feeds STATIC ${ULL_FEEDS_SOURCES})

target_include_directories(ull_feeds
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ull_feeds
    PUBLIC
        ull_common
)

if(ULL_COMPILER_HAS_AVX2)
    target_compile_definitions(ull_feeds PRIVATE ULL_HAS_AVX2_SCANNER=1)
    if(MSVC)
        set_source_files_properties(structural_scan_avx2.cpp PROPERTIES COMPILE_OPTIONS "/arch:AVX2")
    else()
        set_source_files_properties(structural_scan_avx2.cpp PROPERTIES COMPILE_OPTIONS "-mavx2")
    endif()
endif()

if(ULL_COMPILER_HAS_AVX512)
    target_compile_definitions(ull_feeds PRIVATE ULL_HAS_AVX512_SCANNER=1)
    if(MSVC)
        set_source_files_properties(structural_scan_avx512.cpp PROPERTIES COMPILE_OPTIONS "/arch:AVX512")
    else()
        set_source_files_properties(structural_scan_avx512.cpp PROPERTIES COMPILE_OPTIONS "-mavx512f;-mavx512bw")
    endif()
endif()
