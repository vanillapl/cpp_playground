#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <chrono>
#include <vector>
#include <string>
#include <random>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace std::chrono;

// 定义控制台颜色，用于美化输出
namespace Color
{
    const string reset = "\033[0m";
    const string red = "\033[31m";
    const string green = "\033[32m";
    const string yellow = "\033[33m";
    const string blue = "\033[34m";
    const string magenta = "\033[35m";
    const string cyan = "\033[36m";
}

mutex cout_mutex; // 用于保护cout的互斥锁
mutex log_mutex;  // 用于保护日志的互斥锁

// 获取当前时间
string getCurrentTime()
{
    // 获取当前时间
    auto now = system_clock::now();
    // 将当前时间转换为时间戳
    auto now_tm = system_clock::to_time_t(now);
    // 将时间戳转换为本地时间
    auto now_tm_lc = localtime(&now_tm);
    // 创建字符串流
    stringstream ss;
    // 将本地时间格式化为字符串
    ss << put_time(now_tm_lc, "%H:%M:%S");
    // 返回字符串
    return ss.str();
}

// 定义一个log函数，用于记录日志信息
void log(const string &carId, const string &logMessage, const string &color = Color::reset)
{
    // 使用lock_guard对log_mutex进行加锁，保证线程安全
    lock_guard<mutex> lock(log_mutex);
    {
        // 使用lock_guard对cout_mutex进行加锁，保证线程安全
        lock_guard<mutex> cout_lock(cout_mutex);
        // 输出日志信息，包括时间、车辆ID和日志内容
        cout << color << "[" << getCurrentTime() << "] " << carId << ": " << logMessage << Color::reset << endl;
    }
}

class ParkingLot
{
private:
    int capacity;                 // 停车场容量
    counting_semaphore<> spaces; // 停车位信号量
    mutex entrance_mutex;         // 进口互斥锁
    mutex exit_mutex;             // 出口互斥锁
    vector<string> parkedCars;    // 已停车的车辆列表
    mutex parkedCars_mutex;       // 已停车车辆列表的互斥锁

public:
    // 构造函数，初始化停车场容量和停车位信号量
    ParkingLot(int capacity) : capacity(capacity), spaces(capacity)
    {
        cout << "停车场已建立，容量为：" << capacity << endl;
    }

    // 停车
    void enter(const string &carId)
    {
        log(carId, "正在尝试进入停车场", Color::yellow);
        // 获取停车位信号量
        spaces.acquire();
        {
            // 加锁，防止多线程同时进入停车场
            lock_guard<mutex> lock(entrance_mutex);
            log(carId, "已进入停车场", Color::green);

            {
                // 加锁，防止多线程同时修改已停车车辆列表
                lock_guard<mutex> lock(parkedCars_mutex);
                // 将车辆加入已停车车辆列表
                parkedCars.push_back(carId);
                log(carId, "已停车, 剩余停车位: " + to_string(capacity - parkedCars.size()), Color::cyan);
            }
        }
    }

    // 出车
    void exit(const string &carId)
    {
        {
            // 加锁，防止多线程同时离开停车场
            lock_guard<mutex> lock(exit_mutex);
            log(carId, "正在尝试离开停车场", Color::magenta);

            {
                // 加锁，防止多线程同时修改已停车车辆列表
                lock_guard<mutex> lock(parkedCars_mutex);
                // 从已停车车辆列表中移除车辆
                auto it = find(parkedCars.begin(), parkedCars.end(), carId);
                if (it != parkedCars.end())
                {
                    parkedCars.erase(it);
                }
                log(carId, "已离开停车场, 剩余停车位: " + to_string(capacity - parkedCars.size()), Color::red);
            }
        }
        // 释放停车位信号量
        spaces.release();
    }
};

void car(ParkingLot &parkingLot, const string &carId, mt19937 &gen)
{
    // 随机生成停车时长和到达延迟时间
    uniform_int_distribution<int> parkiongDuring(2000, 8000);
    uniform_int_distribution<int> arriveDelay(1000, 3000);

    // 模拟到达延迟时间
    this_thread::sleep_for(milliseconds(arriveDelay(gen)));

    // 车辆进入停车场
    parkingLot.enter(carId);

    // 随机生成停车时长
    int stayTime = parkiongDuring(gen);
    // 模拟停车时长
    this_thread::sleep_for(milliseconds(stayTime));

    // 车辆离开停车场
    parkingLot.exit(carId);
}

int main()
{
    // 创建一个随机数生成器
    random_device rd;
    mt19937 gen(rd());

    // 定义停车场容量
    const int PARKING_LOT_CAPACITY = 5;

    // 创建一个停车场对象
    ParkingLot parkingLot(PARKING_LOT_CAPACITY);

    // 定义车辆数量
    const int CAR_COUNT = 10;
    vector<thread> cars;
    // 创建多个车辆线程
    for (int i = 0; i < CAR_COUNT; ++i)
    {
        cars.emplace_back(car, ref(parkingLot), "Car-" + to_string(i + 1), ref(gen));
    }

    // 等待所有车辆线程结束
    for (auto &car : cars)
    {
        car.join();
    }

    // 输出所有车辆已离开停车场
    cout << "所有车辆已离开停车场" << endl;

    return 0;
}