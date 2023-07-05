#include "SkipList.h"
#include <iostream>

// #define FILE_PATH "../store/dumpFile"

using namespace mySkipList;
int main(int argv, char *argc[]) {

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    SkipList<int, std::string> mySkipList(false);
    mySkipList.insert(1, "南理工");
    mySkipList.insert(3, "北理工");
    mySkipList.insert(5, "南航");
    mySkipList.insert(7, "哈工大");
    mySkipList.insert(9, "北航");

    // 重复键值
    mySkipList.insert(5, "小清华");
    mySkipList.insert(7, "深职院");
    mySkipList.insert(9, "广轻工");

    std::cout << "SkipList's size : " << mySkipList.size() << std::endl;

    mySkipList.dumpFile();

    std::string value;
    mySkipList.get(1, value);
    cout << value << endl;

    mySkipList.get(4, value);
    cout << value << endl;

    mySkipList.display();

    mySkipList.erase(1);
    mySkipList.erase(4);

    std::cout << "SkipList's size : " << mySkipList.size() << std::endl;

    mySkipList.display();
}