#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <tlhelp32.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")

// Class to manage resources and clean up when they go out of scope
class DataToFree {
public:
    HANDLE hProcess;          // Handle to the opened process
    HANDLE snapshotHandle;    // Handle for process snapshot (not used here)
    LPVOID remoteMemoryAddr;  // Address of allocated memory in the remote process
    int remoteMemorySize;     // Size of the allocated memory

    // Constructor initializes handles to nullptr and sizes to zero
    DataToFree() {
        this->hProcess = nullptr;
        this->snapshotHandle = nullptr;
        this->remoteMemoryAddr = nullptr;
        this->remoteMemorySize = 0;
    }

    // Destructor to clean up resources
    ~DataToFree() {
        if (this->hProcess != nullptr) {
            // Free the allocated memory if it exists
            if (this->remoteMemoryAddr != nullptr && this->remoteMemorySize != 0) {
                VirtualFreeEx(this->hProcess, this->remoteMemoryAddr, this->remoteMemorySize, MEM_RELEASE);
                this->remoteMemoryAddr = nullptr;
                this->remoteMemorySize = 0;
            }
            CloseHandle(this->hProcess); // Close the process handle
            this->hProcess = nullptr;
        }

        if (this->snapshotHandle != nullptr) {
            CloseHandle(this->snapshotHandle); // Close the snapshot handle
            this->snapshotHandle = nullptr;
        }
    }
};

// Function to print usage information
void printHelp() {
    std::cout << "Usage: inject_dll.exe <pid> <dll path> [--verbose] [--help]\n"
              << "Injects a DLL into a remote process.\n\n"
              << "Arguments:\n"
              << "  <pid>        The process ID to inject into.\n"
              << "  <dll path>   The path to the DLL to inject.\n"
              << "  --verbose     Enable verbose output.\n"
              << "  --help        Show this help message.\n";
}

int wmain(int argc, wchar_t *argv[], wchar_t *envp[]) {
    bool verbose = false; // Flag for verbose output

    // Check for help and verbose flags
    for (int i = 1; i < argc; ++i) {
        if (_wcsicmp(argv[i], L"--help") == 0) {
            printHelp(); // Show help message
            return 0;
        }
        if (_wcsicmp(argv[i], L"--verbose") == 0) {
            verbose = true; // Enable verbose mode
        }
    }

    // Ensure the correct number of arguments are provided
    if (argc != 3 && argc != 4) {
        std::cout << "Expected 2 arguments (pid, dll name) or 3 arguments (including --verbose)." << std::endl;
        return 1;
    }

    // Print start message if verbose mode is enabled
    if (verbose) {
        std::cout << "Running executable to inject dll." << std::endl;
    }

    // Create an instance of the resource manager
    if (verbose) {
        std::cout << "Creating an instance of resource Manager" << std::endl;
    }
    DataToFree dataToFree;

    // Convert the first argument to a process ID
    const int pid = _wtoi(argv[1]);
    if (pid == 0) {
        std::cout << "Invalid pid." << std::endl;
        return 2; // Return error if PID is invalid
    }

    if (verbose) {
        std::cout << "Parsed PID: " << pid << std::endl; // Log the parsed PID
    }

    const int MAX_PATH_SIZE_PADDED = MAX_PATH + 1; // Max path size for the DLL
    char dllPath[MAX_PATH_SIZE_PADDED]; // Buffer for the DLL path
    memset(&dllPath[0], '\0', MAX_PATH_SIZE_PADDED); // Initialize buffer
    size_t pathLen = 0; // Variable to hold length of the converted path

    // Convert wide string to multibyte string (DLL path)
    if (verbose) {
        std::cout << "Converting wide string to multibyte string (DLL path)" << std::endl;
    }
    wcstombs_s(&pathLen, dllPath, argv[2], MAX_PATH);

    if (verbose) {
        std::cout << "DLL Path: " << dllPath << std::endl; // Log the DLL path
    }

    // Open the target process with necessary permissions
    const bool inheritable = false; // No handle inheritance
    if (verbose) {
        std::cout << "Opening the target process with necessary permissions" << dllPath << std::endl; // Log the DLL path
    }
    const HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, inheritable, pid);
    if (hProcess == nullptr || hProcess == INVALID_HANDLE_VALUE) {
        std::cout << "Unable to open process with pid: " << pid << ". Error code: " << GetLastError() << "." << std::endl;
        return 3; // Return error if unable to open process
    }
    dataToFree.hProcess = hProcess; // Store the process handle

    if (verbose) {
        std::cout << "Successfully opened process with pid: " << pid << std::endl; // Log successful open
    }

    // Allocate memory in the remote process for the DLL path
    if (verbose) {
        std::cout << "Allocating memory in remote process" << std::endl;
    }
    const LPVOID remoteMemoryAddr = VirtualAllocEx(hProcess, nullptr, MAX_PATH_SIZE_PADDED, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (remoteMemoryAddr == nullptr) {
        std::cout << "Error. Unable to allocate memory in pid: " << pid << ". Error code: " << GetLastError() << "." << std::endl;
        return 4; // Return error if memory allocation fails
    }
    dataToFree.remoteMemorySize = MAX_PATH_SIZE_PADDED; // Store allocated memory size
    dataToFree.remoteMemoryAddr = remoteMemoryAddr; // Store allocated memory address

    if (verbose) {
        std::cout << "Allocated memory in remote process at address: " << remoteMemoryAddr << std::endl; // Log memory allocation
    }

    // Write the DLL path to the allocated memory in the remote process
    if (verbose) {
        std::cout << "Writing the DLL path to the allocated memory in the remote process" << std::endl;
    }
    const bool written = WriteProcessMemory(hProcess, remoteMemoryAddr, dllPath, pathLen, nullptr);
    if (!written) {
        std::cout << "Error. Unable to write to memory in pid: " << pid << ". Error code: " << GetLastError() << "." << std::endl;
        return 5; // Return error if writing to memory fails
    }

    if (verbose) {
        std::cout << "Successfully wrote to memory in pid: " << pid << std::endl; // Log successful write
    }

    // Get the address of LoadLibraryA in kernel32.dll
    if (verbose) {
        std::cout << "Getting the address of LoadLibraryA in kernel32.dll" << std::endl;
    }
    const LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    if (loadLibraryAddress == nullptr) {
        std::cout << "Error. Unable to get LoadLibraryA address. Error code: " << GetLastError() << "." << std::endl;
        return 6; // Return error if unable to get function address
    }

    if (verbose) {
        std::cout << "LoadLibraryA address: " << loadLibraryAddress << std::endl; // Log LoadLibraryA address
    }

    // Create a remote thread in the target process to execute LoadLibraryA
    if (verbose) {
        std::cout << "Creating a remote thread in the target process to execute LoadLibraryA" << std::endl;
    }
    const HANDLE remoteThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, remoteMemoryAddr, 0, nullptr);
    if (remoteThread == nullptr) {
        std::cout << "Error. Unable to CreateRemoteThread. Error code: " << GetLastError() << "." << std::endl;
        return 7; // Return error if thread creation fails
    }

    if (verbose) {
        std::cout << "Created remote thread to execute LoadLibraryA." << std::endl; // Log thread creation
    }

    // Wait for the remote thread to finish execution
    if (verbose) {
        std::cout << "Waiting for LoadLibraryA to complete." << std::endl; // Log waiting for completion
    }
    DWORD result = WaitForSingleObject(remoteThread, 5 * 1000); // Wait up to 5 seconds

    // Check the result of the wait
    if (result == WAIT_TIMEOUT) {
        std::cout << "WaitForSingleObject(LoadLibraryA thread) timed out." << std::endl;
        return 8; // Return error if waiting times out
    } else if (result == WAIT_FAILED) {
        std::cout << "WaitForSingleObject(LoadLibraryA thread) failed. Error code: " << GetLastError() << "." << std::endl;
        return 9; // Return error if wait fails
    }

    if (verbose) {
        std::cout << "DLL injection completed successfully." << std::endl; // Log successful injection
    }
    return 0; // Exit with success
}
