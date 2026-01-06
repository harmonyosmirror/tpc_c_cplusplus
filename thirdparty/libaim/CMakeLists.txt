cmake_minimum_required(VERSION 3.5)

project(libaim)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(
    include/
)
set(CLIENT_SOURCES
    flap.cpp
    tlv.cpp
    aimsession.cpp
    defaultaimlistener.cpp
    buddyevent.cpp
)

set(SERVER_SOURCES
    flap.cpp
    tlv.cpp
    aimsession.cpp
    buddyevent.cpp
    defaultaimlistener.cpp
)

add_library(aim SHARED ${CLIENT_SOURCES})

add_library(aim_static STATIC ${CLIENT_SOURCES})


# 客户端demo：
    # jaim.cpp
    # htmlparser.cpp

# 服务端demo：
    # server.cpp

    # 启用测试

# ------------------------------ 本地原生测试配置（条件启用） ------------------------------
option(BUILD_OHOS_TEST "Build test for Linux native (x86_64)" OFF)
if(BUILD_OHOS_TEST)
    enable_testing()
    # 测试源码（无 GTest 依赖）
    add_executable(test_libaim_ohos test_libaim_ohos.cpp)
    target_link_options(test_libaim_ohos PRIVATE
        -Wl,--dynamic-linker=/system/lib/ld-musl-arm.so.1)

    # 链接依赖（鸿蒙系统库 + libaim）
    target_link_libraries(test_libaim_ohos
        aim
        c
        c++
    )

    # 指定测试程序输出目录（与 hpkbuild 中一致）
    set_target_properties(test_libaim_ohos PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
    )

    install(TARGETS test_libaim_ohos DESTINATION bin)
endif()
