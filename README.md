# Hardware Information CLI Tool (v1.1.1)

## Why this tool?
This tool provides a quick, interactive way to check essential hardware and system components of a Windows PC directly from the command line. It features a modern CLI interface with:
- **Visual Enhancements:** ANSI colors, boxed panels for organized output, and simulated loading spinners.
- **System Insights:** OS details, CPU architecture, RAM, GPU specs, and storage usage.
- **Environment Detection:** Identify your active shell and terminal emulator.

## New in v1.1.1
- **Enhanced Commands:** Added `/os`, `/storage`, `/uptime`, and `/check`.
- **UI Overhaul:** All information is now presented in clean, color-coded boxed panels.
- **Improved Build Script:** Updated `build.bat` to include all necessary Windows API libraries for a seamless compilation.

## How to use
1. **Compilation:**
   Run the provided `build.bat` script to compile the tool using Microsoft Visual C++ compiler.
   ```cmd
   build.bat
   ```
2. **Running:**
   After compilation, run `hwcheck.exe` in your terminal.
   ```cmd
   hwcheck.exe
   ```

## Interactive Commands
Once running, you can use the following commands:
- `/all`     - Show a complete scan of all system and hardware information.
- `/os`      - Display Operating System details (Product Name, Build).
- `/core`    - Display CPU architecture and physical RAM.
- `/gpu`     - List installed graphics adapters and dedicated memory.
- `/monitor` - Show connected displays, resolutions, and primary status.
- `/storage` - List fixed drives with total and free space.
- `/uptime`  - Show how long the system has been running.
- `/check`   - Detect and display the active shell and terminal emulator.
- `/clear`   - Clear the console screen.
- `/help`    - Show the help menu.
- `/exit`    - Quit the application.

## Requirements
- Windows OS
- MSVC Build Tools (for compilation)
