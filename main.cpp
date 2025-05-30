#include "UniConv.h"
#include "LightLogWriteImpl.h"
#include <iostream>

// �����µĲ��Ժ���
void RunAllTests();

int main() {
    std::cout << "UniConv ����ת������Գ���" << std::endl;
    std::cout << "============================" << std::endl;
    
    try {
        // �������в���
        RunAllTests();
        
        std::cout << "���в�����ɣ���鿴��־�ļ�������ļ���" << std::endl;
        std::cout << "��־�ļ���log/TestNewConvert.log" << std::endl;
        std::cout << "����ļ���testdata/output/" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "���Թ����з�������" << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
