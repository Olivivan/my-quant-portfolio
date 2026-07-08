add_executable(ull_gateway_app
    main.cpp
)

target_link_libraries(ull_gateway_app
    PRIVATE
        ull_gateway
)
