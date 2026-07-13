set(COMMON_SOURCES
    common/types.cpp
    common/config.cpp
    common/logger.cpp
)

set(ETL_SOURCES
    etl/tick_parser.cpp
    etl/cleaner.cpp
    etl/imputer.cpp
    etl/csv_writer.cpp
)

set(DB_SOURCES
    db/connection_pool.cpp
    db/timescale_writer.cpp
    db/feature_repository.cpp
    db/feature_writer.cpp
    db/time_utils.cpp
    db/redis_cache.cpp
)

set(FEATURE_SOURCES
    features/aggregator.cpp
    features/technical_indicators.cpp
    features/feature_matrix.cpp
)

set(INFERENCE_SOURCES
    inference/onnx_engine.cpp
    inference/signal_generator.cpp
)

add_library(quant_pipeline_lib STATIC
    ${COMMON_SOURCES}
    ${ETL_SOURCES}
    ${DB_SOURCES}
    ${FEATURE_SOURCES}
    ${INFERENCE_SOURCES}
)

target_include_directories(quant_pipeline_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src/cpp
)

target_include_directories(quant_pipeline_lib PUBLIC
    ${PostgreSQL_INCLUDE_DIRS}
    ${LIBPQXX_INCLUDE_DIR}
    ${ONNXRUNTIME_DIR}/include
)

target_link_libraries(quant_pipeline_lib PUBLIC
    Arrow::arrow_shared
    ${LIBPQXX_LIB}
    ${PostgreSQL_LIBRARIES}
    fmt::fmt
    spdlog::spdlog
    TBB::tbb
    ${ONNXRUNTIME_LIB}
    hiredis::hiredis
    nlohmann_json::nlohmann_json
)

add_executable(quant_etl main_etl.cpp)
target_link_libraries(quant_etl PRIVATE quant_pipeline_lib)

add_executable(quant_features main_features.cpp)
target_link_libraries(quant_features PRIVATE quant_pipeline_lib)

add_executable(quant_inference main_inference.cpp)
target_link_libraries(quant_inference PRIVATE quant_pipeline_lib)
