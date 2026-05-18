#include <iostream>
#include <windows.h>
#include <vector>
#include <dxgi.h>
#include <string>

#pragma comment(lib, "dxgi.lib")

// ANSI Color Codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

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

    std::cout << CYAN << BOLD << "\n--- Core Hardware ---" << RESET << std::endl;
    std::cout << YELLOW << "Processor Architecture: " << RESET;
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: std::cout << "x64 (AMD or Intel)"; break;
        case PROCESSOR_ARCHITECTURE_ARM: std::cout << "ARM"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: std::cout << "ARM64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: std::cout << "x86"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << std::endl;
    std::cout << YELLOW << "Number of Processors: " << RESET << sysInfo.dwNumberOfProcessors << std::endl;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        std::cout << YELLOW << "Total Physical Memory: " << RESET << memStatus.ullTotalPhys / (1024 * 1024) << " MB" << std::endl;
    }
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
           |___/   )" << RESET << " " << YELLOW << "v1.1.0" << RESET << "\n";
    std::cout << GREEN << " Welcome to the Interactive Hardware Checker!" << RESET << "\n";
}

void PrintHelp() {
    std::cout << BOLD << "\nAvailable Commands:" << RESET << "\n";
    std::cout << CYAN << "  /all" << RESET << "     - Show all hardware information\n";
    std::cout << CYAN << "  /core" << RESET << "    - Show CPU and RAM information\n";
    std::cout << CYAN << "  /gpu" << RESET << "     - Show Graphics Card information\n";
    std::cout << CYAN << "  /monitor" << RESET << " - Show connected display information\n";
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
            PrintCoreHardware();
            PrintGPUInfo();
            PrintMonitorInfo();
        } else if (input == "/core") {
            PrintCoreHardware();
        } else if (input == "/gpu") {
            PrintGPUInfo();
        } else if (input == "/monitor") {
            PrintMonitorInfo();
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
