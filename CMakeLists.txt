add_library(hnc_utils INTERFACE)

# 将头文件目录添加到接口库中，其他模块使用这个库时会自动包含这些头文件
target_include_directories(hnc_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

