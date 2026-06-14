#include <iostream>
#include <windows.h>
#include <vector>
#include <dxgi.h>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>

#pragma comment(lib, "dxgi.lib")

// ANSI Color Codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

void PrintBox(const std::string& title, const std::vector<std::string>& lines, const std::string& color = CYAN) {
    size_t width = title.length() + 6;
    for (const auto& line : lines) {
        if (line.length() + 6 > width) width = line.length() + 6;
    }

    std::cout << color << " " << std::string(width, '-') << RESET << "\n";
    std::cout << color << "| " << BOLD << title << RESET << std::string(width - title.length() - 2, ' ') << color << " |" << RESET << "\n";
    std::cout << color << "| " << std::string(width - 2, '-') << " |" << RESET << "\n";
    for (const auto& line : lines) {
        std::cout << color << "| " << RESET << line << std::string(width - line.length() - 3, ' ') << color << " |" << RESET << "\n";
    }
    std::cout << color << " " << std::string(width, '-') << RESET << "\n";
}

void SimulateLoading(const std::string& message) {
    const char spinner[] = {'|', '/', '-', '\\'};
    std::cout << YELLOW << message << " ";
    for (int i = 0; i < 12; ++i) {
        std::cout << spinner[i % 4] << " \b\b" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << GREEN << "Done!" << RESET << "\n";
}

// Enable ANSI colors for Windows Terminal
void EnableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

std::string WideToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

void PrintCoreHardware() {
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    std::vector<std::string> lines;

    std::string arch = "Unknown";
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: arch = "x64 (AMD or Intel)"; break;
        case PROCESSOR_ARCHITECTURE_ARM: arch = "ARM"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
    }
    lines.push_back("Processor Architecture: " + arch);
    lines.push_back("Number of Processors:   " + std::to_string(sysInfo.dwNumberOfProcessors));

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        lines.push_back("Total Physical Memory:  " + std::to_string(memStatus.ullTotalPhys / (1024 * 1024)) + " MB");
    }

    PrintBox("Core Hardware", lines, CYAN);
}

void PrintGPUInfo() {
    std::cout << GREEN << BOLD << "\n--- GPU Information ---" << RESET << std::endl;
    IDXGIFactory* pFactory = NULL;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        std::cout << RED << "Failed to create DXGI factory." << RESET << std::endl;
        return;
    }

    IDXGIAdapter* pAdapter;
    for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC desc;
        pAdapter->GetDesc(&desc);

        std::string str = WideToString(desc.Description);
        std::cout << YELLOW << "Adapter " << i << ": " << RESET << str << std::endl;
        std::cout << "  Dedicated Video Memory: " << desc.DedicatedVideoMemory / (1024 * 1024) << " MB" << std::endl;
        
        pAdapter->Release();
    }
    pFactory->Release();
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT /*lprcMonitor*/, LPARAM /*dwData*/) {
    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMonitor, &mi)) {
        std::cout << YELLOW << "Monitor Name: " << RESET << mi.szDevice << std::endl;
        std::cout << "  Resolution: " << (mi.rcMonitor.right - mi.rcMonitor.left) << "x" << (mi.rcMonitor.bottom - mi.rcMonitor.top) << std::endl;
        if (mi.dwFlags & MONITORINFOF_PRIMARY) {
            std::cout << GREEN << "  Primary: Yes" << RESET << std::endl;
        }
    }
    return TRUE;
}

void PrintMonitorInfo() {
    std::cout << MAGENTA << BOLD << "\n--- Monitor Information ---" << RESET << std::endl;
    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0)) {
        std::cout << RED << "Failed to enumerate monitors." << RESET << std::endl;
    }
}

void PrintOSInfo() {
    std::vector<std::string> lines;
    char buffer[256];
    DWORD size = sizeof(buffer);

    // Get OS Name from Registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
            lines.push_back("OS Name:       " + std::string(buffer));
        }
        size = sizeof(buffer);
        if (RegQueryValueExA(hKey, "CurrentBuild", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
            lines.push_back("Build Number:  " + std::string(buffer));
        }
        RegCloseKey(hKey);
    }

    size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        lines.push_back("Computer Name: " + std::string(buffer));
    }

    PrintBox("Operating System", lines, MAGENTA);
}

void PrintStorageInfo() {
    std::vector<std::string> lines;
    DWORD drives = GetLogicalDrives();
    for (char i = 0; i < 26; ++i) {
        if (drives & (1 << i)) {
            char drivePath[] = { (char)('A' + i), ':', '\\', '\0' };
            UINT type = GetDriveTypeA(drivePath);
            if (type == DRIVE_FIXED) {
                ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
                if (GetDiskFreeSpaceExA(drivePath, &freeBytes, &totalBytes, &totalFreeBytes)) {
                    std::string line = std::string(1, 'A' + i) + ": [Fixed] ";
                    line += std::to_string(totalBytes.QuadPart / (1024 * 1024 * 1024)) + " GB Total, ";
                    line += std::to_string(freeBytes.QuadPart / (1024 * 1024 * 1024)) + " GB Free";
                    lines.push_back(line);
                }
            }
        }
    }
    PrintBox("Storage Information", lines, YELLOW);
}

void PrintUptime() {
    ULONGLONG uptimeMS = GetTickCount64();
    ULONGLONG seconds = uptimeMS / 1000;
    ULONGLONG minutes = seconds / 60;
    ULONGLONG hours = minutes / 60;
    ULONGLONG days = hours / 24;

    std::vector<std::string> lines;
    std::string uptimeStr = std::to_string(days) + "d " + std::to_string(hours % 24) + "h " + std::to_string(minutes % 60) + "m " + std::to_string(seconds % 60) + "s";
    lines.push_back("System Uptime: " + uptimeStr);
    PrintBox("Uptime", lines, GREEN);
}

void PrintCheck() {
    std::vector<std::string> lines;
    std::string shell = "Unknown";
    std::string terminal = "Default Console Host";
    bool shellDetected = false;
    bool termDetected = false;

    // Detect Shell
    char envBuf[256];
    if (GetEnvironmentVariableA("PSModulePath", envBuf, sizeof(envBuf)) > 0) {
        shell = "PowerShell";
        shellDetected = true;
    } else if (GetEnvironmentVariableA("BASH", envBuf, sizeof(envBuf)) > 0 || GetEnvironmentVariableA("SHELL", envBuf, sizeof(envBuf)) > 0) {
        shell = "Bash/Unix-like";
        shellDetected = true;
    } else if (GetEnvironmentVariableA("COMSPEC", envBuf, sizeof(envBuf)) > 0) {
        shell = "CMD (Command Prompt)";
        shellDetected = true;
    }

    // Detect Terminal Emulator
    if (GetEnvironmentVariableA("WT_SESSION", envBuf, sizeof(envBuf)) > 0) {
        terminal = "Windows Terminal";
        termDetected = true;
    } else if (GetEnvironmentVariableA("TERM_PROGRAM", envBuf, sizeof(envBuf)) > 0) {
        terminal = envBuf;
        termDetected = true;
    } else if (GetEnvironmentVariableA("TERM", envBuf, sizeof(envBuf)) > 0) {
        terminal = std::string("Terminal (") + envBuf + ")";
        termDetected = true;
    }

    lines.push_back("Active Shell: " + shell);
    lines.push_back("Terminal:     " + terminal);

    std::string headerColor = (shellDetected) ? GREEN : YELLOW;
    PrintBox("Environment Check", lines, headerColor);
}

void PrintBanner() {
    std::cout << CYAN << BOLD;
    std::cout << R"(
  _____  _        _____  _               _    
 |  __ \(_)      / ____|| |             | |   
 | |__) |_  __ _| |     | |__   ___  ___| | __
 |  _  /| |/ _` | |     | '_ \ / _ \/ __| |/ /
 | | \ \| | (_| | |____ | | | |  __/ (__|   < 
 |_|  \_\_|\__, |\_____||_| |_|\___|\___|_|\_\
            __/ |                             
           |___/   )" << RESET << " " << YELLOW << "v1.1.1" << RESET << "\n";
    std::cout << GREEN << " Welcome to the Interactive Hardware Checker!" << RESET << "\n";
}

void PrintHelp() {
    std::cout << BOLD << "\nAvailable Commands:" << RESET << "\n";
    std::cout << CYAN << "  /all" << RESET << "     - Show all system information\n";
    std::cout << CYAN << "  /os" << RESET << "      - Show Operating System details\n";
    std::cout << CYAN << "  /core" << RESET << "    - Show CPU and RAM information\n";
    std::cout << CYAN << "  /gpu" << RESET << "     - Show Graphics Card information\n";
    std::cout << CYAN << "  /monitor" << RESET << " - Show connected display information\n";
    std::cout << CYAN << "  /storage" << RESET << " - Show disk drive information\n";
    std::cout << CYAN << "  /uptime" << RESET << "  - Show system uptime\n";
    std::cout << CYAN << "  /check" << RESET << "   - Detect shell and terminal environment\n";
    std::cout << CYAN << "  /clear" << RESET << "   - Clear the console screen\n";
    std::cout << CYAN << "  /help" << RESET << "    - Show this help menu\n";
    std::cout << RED << "  /exit" << RESET << "    - Close the tool\n";
}

void ClearScreen() {
    system("cls");
}

int main() {
    EnableVTMode();
    PrintBanner();
    
    std::cout << "\nType " << CYAN << "/help" << RESET << " to see available commands or " << RED << "/exit" << RESET << " to quit.\n";

    std::string input;
    while (true) {
        std::cout << "\n" << BOLD << GREEN << "RigCheck> " << RESET;
        std::getline(std::cin, input);

        if (input == "/exit") {
            std::cout << YELLOW << "Goodbye!" << RESET << "\n";
            break;
        } else if (input == "/help") {
            PrintHelp();
        } else if (input == "/all") {
            SimulateLoading("Scanning all system components...");
            PrintOSInfo();
            PrintCoreHardware();
            PrintGPUInfo();
            PrintMonitorInfo();
            PrintStorageInfo();
            PrintUptime();
        } else if (input == "/os") {
            PrintOSInfo();
        } else if (input == "/core") {
            PrintCoreHardware();
        } else if (input == "/gpu") {
            PrintGPUInfo();
        } else if (input == "/monitor") {
            PrintMonitorInfo();
        } else if (input == "/storage") {
            PrintStorageInfo();
        } else if (input == "/uptime") {
            PrintUptime();
        } else if (input == "/check") {
            PrintCheck();
        } else if (input == "/clear") {
            ClearScreen();
            PrintBanner();
        } else if (input.empty()) {
            continue;
        } else {
            std::cout << RED << "Unknown command: '" << input << "'. Type /help for options." << RESET << "\n";
        }
    }

    return 0;
}
