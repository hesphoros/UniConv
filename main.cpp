#include "UniConv.h"
#include "LightLogWriteImpl.h"
#include <iostream>

// 声明新的测试函数
void RunAllTests();

int main() {
    std::cout << "UniConv 编码转换库测试程序" << std::endl;
    std::cout << "============================" << std::endl;
    
    try {
        // 运行所有测试
        RunAllTests();
        
        std::cout << "所有测试完成！请查看日志文件和输出文件。" << std::endl;
        std::cout << "日志文件：log/TestNewConvert.log" << std::endl;
        std::cout << "输出文件：testdata/output/" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "测试过程中发生错误：" << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
