
/* SkipListDB */
#pragma once
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <stdlib.h>
#include <string>
#include <vector>

namespace mySkipList {
const std::string STORE_FILE = "dumpFile";
using namespace std;
template <typename K, typename V>
class SkipList;
template <typename K, typename V>
class Node {
    friend class SkipList<K, V>;

  private:
    K _k;                 // key
    V _v;                 // value
    vector<Node *> _next; // 指向下一个结点，由于有多层索引，所以每层都有一个next，next数组的长度和该结点拥有的层数有关
  public:
    Node(K k, V v, int level) : _k(k), _v(v), _next(level, nullptr) {}
    ~Node() {
#ifdef DEBUG
        cout << "delete" << getKey() << ":" << getValue() << endl;
#endif
    }

    K getKey() const { return _k; }
    V getValue() const { return _v; }
};

template <typename K, typename V>
class SkipList {

  private:
    Node<K, V> *_head;        // 虚拟头结点
    const int _maxLevel = 32; // 用户限制的索引最大层数
    mutex _mutex;             // 用于多线程互斥访问
    int _curLevel;            // 当前索引层数
    int _elementsNum;         // 元素数量
    const bool _isGuard;      // 是否使用互斥锁保证线程安全

    // 文件操作
    ofstream _fileWriter;
    ifstream _fileReader;

  private:
    Node<K, V> *search(K key) {
        Node<K, V> *cur = _head;
        // int level = _head->_next.size(); // 虚拟头结点的next包含了所有层的第一个结点
        int level = _curLevel;

        while (level--) {
            // 在每一层中持续向右遍历，直到下一个结点不存在或者要查询的key大于当前结点的key
            while (cur->_next[level] != nullptr && cur->_next[level]->_k < key) {
                cur = cur->_next[level];
            }
            // 如果key值相等，则直接返回找到的结点
            if (cur->_next[level] != nullptr && cur->_next[level]->_k == key) {
                return cur->_next[level];
            }
            // 当前层没找到，就到下一层继续找
        }
        return nullptr;
    } // 在跳表中搜索key结点并返回
    int roll() const {
        random_device rd;                      // 生成一个随机种子
        default_random_engine generator(rd()); // 生成一个随机数序列

        int level = 1; // 一定要从1开始，因为默认会有原始链表的一层索引
        while (generator() % 2) {
            level++;
        }

        return level > _maxLevel ? _maxLevel : level;
    } // 1/2概率roll骰子决定插入结点需要更新到层数
    void insertWithoutGuard(const K key, const V val) {
        Node<K, V> *node = search(key);

        if (node != nullptr) {
            node->_v = val;
            return;
        }

        // roll出插入结点后需要更新的索引高度
        int newLevel = roll();

        // 新结点高度超出当前跳表的索引高度，那么就新增一层
        // int curLevel = _head->_next.size();
        int curLevel = _curLevel;
        if (newLevel > curLevel) {
            _head->_next.push_back(nullptr);
            curLevel++;
            _curLevel = curLevel;
        }
        // 创建新结点
        _elementsNum++;
        node = new Node<K, V>(key, val, newLevel);

        // 从最高层出发更新结点
        Node<K, V> *cur = _head;
        while (curLevel--) {
            // 向右遍历，直到右侧结点不存在或者大于key
            while (cur->_next[curLevel] != nullptr && cur->_next[curLevel]->_k < key) {
                cur = cur->_next[curLevel];
            }

            if (curLevel < newLevel) {
                // 如果当前层是需要插入索引的层则插入
                // 例如curLevel=3，newLevel=2，curLevel进入循环后变为2，条件不成立，即第三层不插入新结点
                // 同理第二层和第一层会插入新结点
                // 调整指针关系，完成结点插入
                node->_next[curLevel] = cur->_next[curLevel];
                cur->_next[curLevel] = node;
            }
        }
    }
    void eraseWithoutGuard(const K key) {
        // 搜索key是否存在，如果不存在，则直接返回
        // Node<K, V> *node = search(key);
        // if (node == nullptr) {
        //     return;
        // }
        // 从最高层开始删除
        // int curLevel = _head->_next.size();
        int curLevel = _curLevel;
        Node<K, V> *cur = _head;
        Node<K, V> *tmp = nullptr; // 记录被删除的结点
        _elementsNum--;
        while (curLevel--) {
            // 向右遍历直到右侧结点大于等于key
            while (cur->_next[curLevel] != nullptr && cur->_next[curLevel]->_k < key) {
                cur = cur->_next[curLevel];
            }
            // 右侧结点不存在或者大于key，则说明本层没有找到该结点，直接跳过到下一层
            if (cur->_next[curLevel] == nullptr || cur->_next[curLevel]->_k > key) {
                continue;
            }
            // 说明找到了本层的key结点，删除,注意每层都要删除
            tmp = cur->_next[curLevel];
            cur->_next[curLevel] = cur->_next[curLevel]->_next[curLevel];
            // 计算需要删掉的索引层，由于上层存在的结点在下一层一定存在，所以可以使用pop_back
            if (_head->_next[curLevel] == nullptr) {
                _head->_next.pop_back();
                _curLevel--;
            }
        }
        // 析构结点
        if (tmp != nullptr) {
            delete tmp;
        }
    }
    bool getWithOutGuard(K key, V &val) {
        Node<K, V> *node = search(key);
        if (node == nullptr) {
            return false;
        } else {
            val = node->_v;
            return true;
        }
    }
    void displayWithoutGuard() const {
#ifdef DEBUG
        std::cout << "display skipList : " << std::endl;
#endif
        int curLevel = _curLevel;
        while (curLevel--) {
#ifdef DEBUG
            std::cout << "Level : " << curLevel + 1 << ':' << std::endl;
#endif
            Node<K, V> *cur = _head->_next[curLevel];
            while (cur != nullptr) {
#ifdef DEBUG
                cout << cur->getKey() << ':' << cur->getValue() << ' ';
#endif
                cur = cur->_next[curLevel];
            }
#ifdef DEBUG
            std::cout << std::endl;
#endif
        }
    }
    void dumpFileWithoutGuard() {
#ifdef DEBUG
        cout << "dumpFIle -------------" << endl;
#endif
        if (!_fileWriter.is_open()) {
            _fileWriter.open(STORE_FILE, ios::out | ios::trunc); // 打开文件，out以输出方式打开，trunc，截断方式打开，如果文件中有内容就清空
        }
        Node<K, V> *cur = _head->_next[0];
        while (cur != nullptr) {
            _fileWriter << cur->getKey() << ":" << cur->getValue() << "\n";
#ifdef DEBUG
            std::cout << cur->getKey() << ":" << cur->getValue() << ";\n";
#endif
            cur = cur->_next[0];
        }
        _fileWriter.flush();
        _fileWriter.close();
    }

  public:
    SkipList(bool isGuard) : _curLevel(1), _elementsNum(0), _isGuard(isGuard) {
        K k;
        V v;
        _head = new Node<K, V>(k, v, 1);
        _head->_next.push_back(nullptr);
    }
    ~SkipList() {
#ifdef DEBUG
        cout << "delete SkipListDB-------------" << endl;
#endif
        Node<K, V> *cur = _head->_next[0];
        while (cur != nullptr) {
            Node<K, V> *tmp = cur;
            cur = cur->_next[0];
            // _elementsNum--;
            delete tmp;
        }
        delete _head;
#ifdef DEBUG
        cout << "delete sucessfully!" << endl;
#endif
    }
    /* 跳表基础操作 */
    // 根据key获得数据
    bool get(K key, V &val) { // 调用search搜索key，如果结点存在就更新val返回true,否则直接返回false
        if (_isGuard) {
            unique_lock<mutex> lck(_mutex);
            return getWithOutGuard(key, val);
        } else {
            return getWithOutGuard(key, val);
        }
    } // 根据key读取value，成功则返回true
    // 插入数据
    void insert(const K key, const V val) { // 加入kv对已经存在，则对值进行更新并返回
        if (_isGuard) {
            unique_lock<mutex> lck(_mutex);
            insertWithoutGuard(key, val);
        } else {
            insertWithoutGuard(key, val);
        }

    } // 插入结点，如果结点存在则更新结点
    // 删除数据
    void erase(const K key) {
        if (_isGuard) {
            unique_lock<mutex> lck(_mutex);
            eraseWithoutGuard(key);
        } else {
            eraseWithoutGuard(key);
        }

    } // 删除结点
    // 输出跳表
    void display() const {
        if (_isGuard) {
            unique_lock<mutex> lck(_mutex);
            displayWithoutGuard();
        } else {
            displayWithoutGuard();
        }
    }
    // 获取跳表当前索引大小
    int size() const {
        if (_isGuard) {
            unique_lock<mutex> lck(_mutex);
            return _elementsNum;
        } else {
            return _elementsNum;
        }
    }
    /* 文件相关操作 */

    // 将数据导出到文件
    void dumpFile() {
        if (_isGuard) {
            unique_lock<mutex> lck(_mutex);
            dumpFileWithoutGuard();
        } else {
            dumpFileWithoutGuard();
        }
    }

    // 从磁盘读取数据并插入数据库中
    void loadFile() {
#ifdef DEBUG
        std::cout << "loadFile ---------------- " << std::endl;
#endif
        _fileReader.open(STORE_FILE, ios::in);
        string line;
        while (getline(_fileReader, line)) {
            // TODO 读入string形式的内容，并以string形式写入数据库
        }
    }
};
} // namespace mySkipList
