#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <cstring>
using namespace std;

// 清除输入缓冲区
void clear_screen() {
    system("clear");
}

// ========== 内存信息 ==========
void show_memory() {
    ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        cerr << "无法读取内存信息" << endl;
        return;
    }
    string line;
    cout << "\n--- 内存信息 ---" << endl;
    while (getline(file, line)) {
        if (line.find("MemTotal") != string::npos ||
            line.find("MemFree") != string::npos ||
            line.find("MemAvailable") != string::npos ||
            line.find("SwapTotal") != string::npos ||
            line.find("SwapFree") != string::npos) {
            cout << line << endl;
        }
    }
    file.close();
}

// ========== CPU 负载 ==========
void show_cpu_load() {
    ifstream file("/proc/loadavg");
    if (!file.is_open()) {
        cerr << "无法读取负载信息" << endl;
        return;
    }
    string line;
    getline(file, line);
    cout << "\n--- CPU 负载 ---" << endl;
    cout << "1分钟 / 5分钟 / 15分钟: " << line << endl;
    file.close();
}

// ========== CPU 信息 ==========
void show_cpu_info() {
    ifstream file("/proc/cpuinfo");
    if (!file.is_open()) {
        cerr << "无法读取 CPU 信息" << endl;
        return;
    }
    string line;
    string model, cores;
    int core_count = 0;
    
    while (getline(file, line)) {
        if (line.find("model name") != string::npos) {
            size_t pos = line.find(":");
            if (pos != string::npos) {
                model = line.substr(pos + 2);
            }
        }
        if (line.find("processor") != string::npos) {
            core_count++;
        }
    }
    file.close();
    
    cout << "\n--- CPU 信息 ---" << endl;
    if (!model.empty()) {
        cout << "型号: " << model << endl;
    }
    cout << "核心数: " << core_count << endl;
}

// ========== 磁盘使用情况 ==========
void show_disk_usage() {
    cout << "\n--- 磁盘使用情况 ---" << endl;
    cout << "Filesystem      Size  Used Avail Use% Mounted on" << endl;
    // 使用系统命令 df -h 读取磁盘信息
    FILE* fp = popen("df -h / /data /storage 2>/dev/null | tail -n +2", "r");
    if (!fp) {
        cerr << "无法读取磁盘信息" << endl;
        return;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        cout << buffer;
    }
    pclose(fp);
}

// ========== 系统运行时间 ==========
void show_uptime() {
    ifstream file("/proc/uptime");
    if (!file.is_open()) {
        cerr << "无法读取运行时间" << endl;
        return;
    }
    double uptime_seconds;
    file >> uptime_seconds;
    file.close();
    
    int days = uptime_seconds / 86400;
    int hours = (int(uptime_seconds) % 86400) / 3600;
    int minutes = (int(uptime_seconds) % 3600) / 60;
    
    cout << "\n--- 系统运行时间 ---" << endl;
    cout << days << " 天 " << hours << " 小时 " << minutes << " 分钟" << endl;
}

// ========== 主菜单 ==========
int main() {
    int choice;
    do {
        cout << "\n=====================" << endl;
        cout << "   手机系统信息工具   " << endl;
        cout << "=====================" << endl;
        cout << "1. 查看内存信息" << endl;
        cout << "2. 查看 CPU 负载" << endl;
        cout << "3. 查看 CPU 信息" << endl;
        cout << "4. 查看磁盘使用" << endl;
        cout << "5. 查看运行时间" << endl;
        cout << "6. 查看全部信息" << endl;
        cout << "0. 退出" << endl;
        cout << "请选择: ";
        cin >> choice;
        
        switch (choice) {
            case 1: show_memory(); break;
            case 2: show_cpu_load(); break;
            case 3: show_cpu_info(); break;
            case 4: show_disk_usage(); break;
            case 5: show_uptime(); break;
            case 6:
                show_memory();
                show_cpu_load();
                show_cpu_info();
                show_disk_usage();
                show_uptime();
                break;
            case 0:
                cout << "已退出。" << endl;
                break;
            default:
                cout << "无效选项，请重新选择。" << endl;
        }
    } while (choice != 0);
    
    return 0;
}
