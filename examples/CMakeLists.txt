# ============================================================================
# 示例程序构建配置
# ============================================================================

# 添加可执行文件
add_executable(simple_demo simple_demo.cpp)
add_executable(stage1_demo stage1_demo.cpp)
add_executable(branch_prediction_demo branch_prediction_demo.cpp)
add_executable(lru_cache_demo lru_cache_demo.cpp)
add_executable(zero_copy_demo zero_copy_demo.cpp)
add_executable(parallel_batch_demo parallel_batch_demo.cpp)
add_executable(instance_usage_demo instance_usage_demo.cpp)
add_executable(encoding_validation_test encoding_validation_test.cpp)

# 链接UniConv库
target_link_libraries(simple_demo PRIVATE UniConv)
target_link_libraries(stage1_demo PRIVATE UniConv)
target_link_libraries(branch_prediction_demo PRIVATE UniConv)
target_link_libraries(lru_cache_demo PRIVATE UniConv)
target_link_libraries(zero_copy_demo PRIVATE UniConv)
target_link_libraries(parallel_batch_demo PRIVATE UniConv)
target_link_libraries(instance_usage_demo PRIVATE UniConv)
target_link_libraries(encoding_validation_test PRIVATE UniConv)

# 设置输出目录
set_target_properties(
    simple_demo 
    stage1_demo 
    branch_prediction_demo 
    lru_cache_demo 
    zero_copy_demo 
    parallel_batch_demo
    instance_usage_demo
    encoding_validation_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)
