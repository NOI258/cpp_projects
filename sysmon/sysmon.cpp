#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>

using namespace std;

// ===== 读取第一行 /proc/stat 的 CPU 时间 =====
vector<unsigned long long> get_cpu_times() {
    ifstream file("/proc/stat");
    if (!file.is_open()) return {};
    string line;
    getline(file, line);
    file.close();
    istringstream iss(line);
    string cpu_label;
    vector<unsigned long long> times(8);
    iss >> cpu_label;
    for (int i = 0; i < 8; ++i) iss >> times[i];
    return times;
}

// ===== 计算 CPU 使用率（%） =====
double get_cpu_usage() {
    static vector<unsigned long long> prev = get_cpu_times();
    if (prev.empty()) return 0.0;
    this_thread::sleep_for(chrono::milliseconds(200));
    vector<unsigned long long> curr = get_cpu_times();
    if (curr.empty()) return 0.0;

    unsigned long long prev_idle = prev[3] + prev[4];   // idle + iowait
    unsigned long long curr_idle = curr[3] + curr[4];
    unsigned long long prev_total = 0, curr_total = 0;
    for (int i = 0; i < 8; ++i) { prev_total += prev[i]; curr_total += curr[i]; }

    double delta_idle = curr_idle - prev_idle;
    double delta_total = curr_total - prev_total;
    prev = curr;
    return (1.0 - delta_idle / delta_total) * 100.0;
}

// ===== 内存信息 =====
string get_mem_info() {
    ifstream file("/proc/meminfo");
    if (!file.is_open()) return "";
    string line, mem_total_str, mem_avail_str;
    unsigned long long total = 0, avail = 0;
    while (getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            istringstream iss(line);
            string label; iss >> label >> total;
        } else if (line.find("MemAvailable:") == 0) {
            istringstream iss(line);
            string label; iss >> label >> avail;
        }
    }
    file.close();
    if (total == 0) return "";
    double used_gb = (total - avail) / 1024.0 / 1024.0;
    double total_gb = total / 1024.0 / 1024.0;
    double percent = (total - avail) * 100.0 / total;
    char buf[64];
    snprintf(buf, sizeof(buf), "%.1f / %.1f GB (%.0f%%)", used_gb, total_gb, percent);
    return string(buf);
}

// ===== 磁盘使用（根目录） =====
string get_disk_usage() {
    FILE* fp = popen("df -h / | tail -1", "r");
    if (!fp) return "";
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp) == nullptr) {
        pclose(fp); return "";
    }
    pclose(fp);
    string line = buffer;
    // 格式: filesystem size used avail use% mount
    // 提取第2、3、5列
    istringstream iss(line);
    string fs, size, used, avail, use_pct, mount;
    iss >> fs >> size >> used >> avail >> use_pct >> mount;
    return used + " / " + size + " (" + use_pct + ")";
}

// ===== 进程数 =====
int get_process_count() {
    int cnt = 0;
    FILE* fp = popen("ps aux | wc -l", "r");
    if (!fp) return 0;
    char buffer[32];
    if (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        cnt = atoi(buffer);
    }
    pclose(fp);
    return cnt;
}

// ===== 清屏 =====
void clear_screen() {
    system("clear");
}

// ===== 主循环 =====
int main() {
    cout << "===== 系统监控仪表盘 (按 Ctrl+C 退出) =====" << endl;
    while (true) {
        clear_screen();
        cout << "\033[1;32m===== 系统监控仪表盘 =====\033[0m" << endl;
        cout << "\n";
        cout << "  CPU 使用率 : " << get_cpu_usage() << " %" << endl;
        cout << "  内存使用   : " << get_mem_info() << endl;
        cout << "  磁盘使用 (/) : " << get_disk_usage() << endl;
        cout << "  运行进程数 : " << get_process_count() << endl;
        cout << "\n";
        cout << "  [按 Ctrl+C 退出]" << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
    return 0;
}
