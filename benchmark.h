#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK(x) std::cerr<<#x ": "<<std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now()-bench_##x).count()<<"s\n";

#include <iostream>
#include <chrono>
#include <thread>

// ANSI 颜色代码
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_CYAN    "\033[1;36m"

// 获取当前线程 ID
#define THREAD_ID std::this_thread::get_id()

// 记录时间戳
#define TICK(x) \
auto bench_##x = std::chrono::steady_clock::now(); \
std::cout << COLOR_GREEN "[BENCHMARK] " COLOR_CYAN << #x " started...\n";

// 计算耗时并打印
#define TOCK(x) \
{ \
auto end_##x = std::chrono::steady_clock::now(); \
double duration_##x = std::chrono::duration_cast<std::chrono::duration<double>>(end_##x - bench_##x).count(); \
std::cout << COLOR_YELLOW " took " << COLOR_RED << duration_##x << "s(" << duration_##x * 1000 << "ms)\n" COLOR_RESET \
<< COLOR_GREEN "[BENCHMARK] " COLOR_CYAN << #x COLOR_RESET << " (Thread: " << THREAD_ID << ")\n\n"; \
}
