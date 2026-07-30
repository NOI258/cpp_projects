#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <sys/stat.h>
#include <ctime>
namespace fs = std::filesystem;

std::string format_size(std::uintmax_t size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double dsize = static_cast<double>(size);
    while (dsize >= 1024 && i < 4) {
        dsize /= 1024.0;
        i++;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << dsize << " " << units[i];
    return oss.str();
}

char get_type(const fs::directory_entry& entry) {
    if (fs::is_directory(entry)) return 'D';
    else if (fs::is_symlink(entry)) return 'L';
    else if (fs::is_regular_file(entry)) return 'F';
    else return '?';
}

std::string get_mtime_str(const fs::path& p) {
    struct stat st;
    if (stat(p.c_str(), &st) == 0) {
        std::time_t t = st.st_mtime;
        std::string s = std::ctime(&t);
        if (!s.empty()) s.pop_back();
        return s;
    }
    return "未知";
}

void list_directory(const std::string& path_str) {
    fs::path path = path_str.empty() ? fs::current_path() : fs::absolute(fs::path(path_str));
    if (!fs::exists(path) || !fs::is_directory(path)) {
        std::cerr << "错误: 路径无效或不是目录\n";
        return;
    }
    std::cout << "\n目录: " << path.string() << "\n";
    std::cout << std::left << std::setw(4) << "类型" 
              << std::setw(20) << "文件名" 
              << std::setw(12) << "大小" 
              << "修改时间\n";
    std::cout << "-----------------------------------------------------------\n";
    for (const auto& entry : fs::directory_iterator(path)) {
        try {
            std::string fname = entry.path().filename().string();
            std::string mtime = get_mtime_str(entry.path());
            std::string size_str = (fs::is_regular_file(entry) ? format_size(fs::file_size(entry)) : "<DIR>");
            std::cout << std::left << std::setw(4) << get_type(entry)
                      << std::setw(20) << fname
                      << std::setw(12) << size_str
                      << mtime << "\n";
        } catch(...) {
            continue;
        }
    }
}

void change_directory(const std::string& path_str) {
    try {
        fs::path newpath = path_str.empty() ? fs::current_path() : fs::absolute(fs::path(path_str));
        if (fs::exists(newpath) && fs::is_directory(newpath)) {
            fs::current_path(newpath);
            std::cout << "当前目录: " << fs::current_path().string() << "\n";
        } else {
            std::cerr << "错误: 目录不存在\n";
        }
    } catch(...) {
        std::cerr << "错误: 无法切换目录\n";
    }
}

void show_info(const std::string& path_str) {
    fs::path p = fs::absolute(fs::path(path_str));
    if (!fs::exists(p)) {
        std::cerr << "错误: 文件不存在\n";
        return;
    }
    struct stat st;
    stat(p.c_str(), &st);
    std::cout << "\n文件信息: " << p.string() << "\n";
    std::cout << "  类型: " << (fs::is_directory(p) ? "目录" : (fs::is_symlink(p) ? "符号链接" : "普通文件")) << "\n";
    if (fs::is_regular_file(p)) {
        std::cout << "  大小: " << format_size(fs::file_size(p)) << "\n";
    }
    std::cout << "  修改时间: " << std::ctime(&st.st_mtime);
}

void ext_stat() {
    std::map<std::string, std::pair<int, std::uintmax_t>> stats;
    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        if (fs::is_regular_file(entry)) {
            std::string ext = entry.path().extension().string();
            if (ext.empty()) ext = "(无扩展名)";
            stats[ext].first++;
            stats[ext].second += fs::file_size(entry);
        }
    }
    if (stats.empty()) {
        std::cout << "当前目录下没有文件。\n";
        return;
    }
    std::cout << "\n扩展名统计:\n";
    std::cout << std::left << std::setw(20) << "扩展名" 
              << std::setw(10) << "数量" 
              << "总大小\n";
    for (const auto& [ext, data] : stats) {
        std::cout << std::left << std::setw(20) << ext
                  << std::setw(10) << data.first
                  << format_size(data.second) << "\n";
    }
}

void copy_item(const std::string& src, const std::string& dst) {
    try {
        fs::path srcp = fs::absolute(fs::path(src));
        fs::path dstp = fs::absolute(fs::path(dst));
        if (!fs::exists(srcp)) {
            std::cerr << "错误: 源不存在\n";
            return;
        }
        if (fs::is_directory(srcp)) {
            fs::copy(srcp, dstp, fs::copy_options::recursive);
        } else {
            fs::copy(srcp, dstp);
        }
        std::cout << "复制成功: " << src << " -> " << dst << "\n";
    } catch(const std::exception& e) {
        std::cerr << "复制失败: " << e.what() << "\n";
    }
}

void move_item(const std::string& src, const std::string& dst) {
    try {
        fs::rename(fs::absolute(fs::path(src)), fs::absolute(fs::path(dst)));
        std::cout << "移动成功: " << src << " -> " << dst << "\n";
    } catch(const std::exception& e) {
        std::cerr << "移动失败: " << e.what() << "\n";
    }
}

void remove_item(const std::string& path_str) {
    try {
        fs::path p = fs::absolute(fs::path(path_str));
        if (!fs::exists(p)) {
            std::cerr << "错误: 文件不存在\n";
            return;
        }
        if (fs::is_directory(p) && !fs::is_empty(p)) {
            std::cerr << "错误: 目录非空，拒绝删除\n";
            return;
        }
        fs::remove(p);
        std::cout << "删除成功: " << p.string() << "\n";
    } catch(const std::exception& e) {
        std::cerr << "删除失败: " << e.what() << "\n";
    }
}

void print_help() {
    std::cout << "\n命令列表:\n"
              << "  ls [path]           列出目录内容\n"
              << "  cd [path]           切换目录\n"
              << "  info <file>         显示文件信息\n"
              << "  extstat             统计当前目录扩展名\n"
              << "  copy <src> <dst>    复制文件或目录\n"
              << "  move <src> <dst>    移动/重命名\n"
              << "  remove <file>       删除文件或空目录\n"
              << "  quit / exit         退出\n"
              << "  help                显示此帮助\n";
}

int main() {
    std::cout << "=== 简易文件管理器 (C++17) ===\n";
    std::cout << "当前目录: " << fs::current_path().string() << "\n";
    std::string line;
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, line);
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "quit" || cmd == "exit") break;
        else if (cmd == "help") { print_help(); continue; }
        else if (cmd == "ls") {
            std::string path;
            iss >> path;
            list_directory(path);
        } else if (cmd == "cd") {
            std::string path;
            iss >> path;
            change_directory(path);
        } else if (cmd == "info") {
            std::string path;
            iss >> path;
            if (path.empty()) std::cerr << "请指定文件路径\n";
            else show_info(path);
        } else if (cmd == "extstat") {
            ext_stat();
        } else if (cmd == "copy") {
            std::string src, dst;
            iss >> src >> dst;
            if (src.empty() || dst.empty()) std::cerr << "用法: copy <源> <目标>\n";
            else copy_item(src, dst);
        } else if (cmd == "move") {
            std::string src, dst;
            iss >> src >> dst;
            if (src.empty() || dst.empty()) std::cerr << "用法: move <源> <目标>\n";
            else move_item(src, dst);
        } else if (cmd == "remove") {
            std::string path;
            iss >> path;
            if (path.empty()) std::cerr << "用法: remove <文件或空目录>\n";
            else remove_item(path);
        } else {
            std::cerr << "未知命令: " << cmd << " (输入 help 查看帮助)\n";
        }
    }
    std::cout << "再见!\n";
    return 0;
}
