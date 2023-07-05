/** 对SkipList进行压力测试
 * 单线程压力测试
 * 插入数据
 *      10万条数据： 0.231257s  QPS: 432419
 *      50万条数据： 1.4096s  QPS: 354711
 *      100万条数据：3.09968s  QPS: 322614
 * 读取数据
 *      10万条数据： 0.0599488s  QPS: 1.66809e+06
 *      50万条数据： 0.743302s  QPS: 672674
 *      100万条数据： 2.00033s  QPS: 499918
 * 多线程压力测试,设置为双线程
 * 插入数据
 *      5*2万条数据： 0.342143s  QPS: 292275
 *      25*2万条数据：1.91207s  QPS: 261496
 *      50*2万条数据：4.12218s  QPS: 242590
 * 读取数据
 *      5*2万条数据： 1.10422s  QPS: 452808
 *      25*2万条数据： 1.11354s  QPS: 449020
 *      50*2万条数据： 2.79907s  QPS: 357261
 * 读取数据

 */
#include "SkipList.h"
#include <chrono>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
using namespace std;
using namespace mySkipList;
using testSkipList = SkipList<int, string>;
const int NUMDATAS = 5 * 1e4;
const int THREADNUM = 2;
random_device rd;
default_random_engine generator(rd());
class oneThreadTestInsertFunc {
  private:
    // mutex _mutex;
    testSkipList *_sl;
    int _cnt; // 测试数据量

  public:
    oneThreadTestInsertFunc(testSkipList *sl, int cnt) : _sl(sl), _cnt(cnt) {}
    void operator()() {
        int i = _cnt;
        while (i--) {
            // unique_lock<mutex> lck(_mutex);
            _sl->insert(generator(), "a");
        }
    }
};

void oneThreadTestInsert(testSkipList *tsl) {
    auto start = chrono::high_resolution_clock::now();
    thread t1(oneThreadTestInsertFunc(tsl, NUMDATAS));
    t1.join();
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "insert costs: " << elapsed.count() << "s  QPS: " << NUMDATAS / elapsed.count() << std::endl;
}

class oneThreadTestGetFunc {
  private:
    // mutex _mutex;
    testSkipList *_sl;
    int _cnt; // 测试数据量

  public:
    oneThreadTestGetFunc(testSkipList *sl, int cnt) : _sl(sl), _cnt(cnt) {}
    void operator()() {
        int i = _cnt;
        while (i--) {
            // unique_lock<mutex> lck(_mutex);
            string tmp;
            _sl->get(generator(), tmp);
        }
    }
};

void oneThreadTestGet(testSkipList *tsl) {
    auto start = chrono::high_resolution_clock::now();
    thread t1(oneThreadTestGetFunc(tsl, NUMDATAS));
    t1.join();
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "get costs: " << elapsed.count() << "s  QPS: " << NUMDATAS / elapsed.count() << std::endl;
}

class multiThreadTestInsertFunc {
  private:
    testSkipList *_sl;
    int _cnt; // 测试数据量

  public:
    multiThreadTestInsertFunc(testSkipList *sl, int cnt) : _sl(sl), _cnt(cnt) {}
    void operator()() {
        int i = _cnt;
        while (i--) {
            _sl->insert(generator(), "a");
        }
    }
};

void multiThreadTestInsert(testSkipList *tsl) {
    auto start = chrono::high_resolution_clock::now();
    vector<thread> threads;
    for (int i = 0; i < THREADNUM; i++) {
        threads.push_back(thread(multiThreadTestInsertFunc(tsl, NUMDATAS)));
    }
    for (int i = 0; i < THREADNUM; i++) {
        threads[i].join();
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "insert costs: " << elapsed.count() << "s  QPS: " << NUMDATAS * THREADNUM / elapsed.count() << std::endl;
}

class multiThreadTestGetFunc {
  private:
    testSkipList *_sl;
    int _cnt; // 测试数据量

  public:
    multiThreadTestGetFunc(testSkipList *sl, int cnt) : _sl(sl), _cnt(cnt) {}
    void operator()() {
        int i = _cnt;
        while (i--) {
            string tmp;
            _sl->get(generator(), tmp);
        }
    }
};

void multiThreadTestGet(testSkipList *tsl) {
    auto start = chrono::high_resolution_clock::now();
    vector<thread> threads;
    for (int i = 0; i < THREADNUM; i++) {
        threads.push_back(thread(multiThreadTestGetFunc(tsl, NUMDATAS)));
    }
    for (int i = 0; i < THREADNUM; i++) {
        threads[i].join();
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "get costs: " << elapsed.count() << "s  QPS: " << NUMDATAS * THREADNUM / elapsed.count() << std::endl;
}

int main(int argc, char const *argv[]) {
    testSkipList tsl(true);

    /* 单线程测试 */
    // oneThreadTestInsert(&tsl);
    // oneThreadTestGet(&tsl);

    /* 多线程测试 */
    multiThreadTestInsert(&tsl);
    multiThreadTestGet(&tsl);

    tsl.dumpFile();
    return 0;
}
