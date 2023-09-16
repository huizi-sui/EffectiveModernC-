#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>

// 单生产者多消费者

static const int kItemRepositorySize = 4; // Item buffer size
static const int kItemsToProduce = 10; // How many items we plan to produce.

class ItemRepository {
public:
    // 产品缓冲区，配合read_position和write_position形成环形队列
    int item_buffer[kItemRepositorySize];
    // 消费者读取产品位置
    size_t read_position;
    // 生产者写入产品位置
    size_t write_position;
    // 互斥量，保护产品缓冲区
    std::mutex mtx;
    // 条件变量，指示产品缓冲区不为满
    std::condition_variable repo_not_full;
    // 条件变量，指示产品缓冲区不为空
    std::condition_variable repo_not_empty;

    // 维护消费者取走产品的计数器
    size_t item_counter;
    std::mutex item_counter_mtx;
};

ItemRepository gItemRepository; // 产品库全局变量，生产者和消费者操作该变量

void ProduceItem(ItemRepository& ir, int item) {
    std::unique_lock<std::mutex> lck(ir.mtx);
    while((ir.write_position + 1) % kItemRepositorySize == ir.read_position) {
        std::cout << "Producer is waiting for an empty slot...\n";
        // 生产者需要等待产品缓冲区不为满这一条件变量
        (ir.repo_not_full).wait(lck);
    } 
    // 生产产品
    (ir.item_buffer)[ir.write_position] = item;
    ir.write_position++;

    if(ir.write_position == kItemRepositorySize) {
        ir.write_position = 0;
    }
    // 通知所有消费者，产品库不为空
    ir.repo_not_empty.notify_all();
    // 解锁
    lck.unlock();
}

int ConsumeItem(ItemRepository& ir) {
    int data;
    std::unique_lock<std::mutex> lck(ir.mtx);
    while(ir.read_position == ir.write_position) {
        std::cout << "Comsumer is waiting for items...\n";
        // 消费者等待生产者生产产品
        ir.repo_not_empty.wait(lck);
    }
    // 读取产品
    data = ir.item_buffer[ir.read_position];
    ir.read_position++;

    if(ir.read_position >= kItemRepositorySize) {
        ir.read_position = 0;
    }

    // 通知生产者产品库还可以继续生产产品
    ir.repo_not_full.notify_all();
    // 解锁
    lck.unlock();
    // 返回产品
    return data;
}
// 生产者任务
void ProducerTask() {
    for(int i = 1; i <= kItemsToProduce; ++i) {
        std::cout << "Produce the " << i << "^th item..." << std::endl;
        ProduceItem(gItemRepository, i);
    }
}
// 消费者任务
void ConsumerTask() {
    bool ready_to_exit = false;
    while(1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // 消费一个产品
        // 上锁
        std::unique_lock<std::mutex> lck(gItemRepository.item_counter_mtx);
        if(gItemRepository.item_counter < kItemsToProduce) {
            int item = ConsumeItem(gItemRepository);
            gItemRepository.item_counter++;
            std::cout << "Consumer thread " << std::this_thread::get_id() << 
                        " is consuming the " << item << "^th item" << std::endl;
            
        } else {
            ready_to_exit = true;
        }
        // 解锁
        lck.unlock();
        if(ready_to_exit) {
            return;
        }
    }
}

void InitItemRepository(ItemRepository& ir) {
    ir.read_position = 0;
    ir.write_position = 0;
    ir.item_counter = 0;
}

int main() {
    InitItemRepository(gItemRepository);
    // 生产者进程
    std::thread producer(ProducerTask);
    // 消费者进程
    std::thread consumer1(ConsumerTask);
    std::thread consumer2(ConsumerTask);
    std::thread consumer3(ConsumerTask);
    std::thread consumer4(ConsumerTask);

    producer.join();
    consumer1.join();
    consumer2.join();
    consumer3.join();
    consumer4.join();
}