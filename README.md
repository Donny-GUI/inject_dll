# 🛠️ DLL Injector

This project contains a C++ application that injects a DLL into a remote process on a Windows system. The injector utilizes Windows API functions to allocate memory, write to the process's memory, and create a remote thread to execute `LoadLibraryA`, which loads the specified DLL.

## 🚀 Features

- **DLL Injection**: Inject a DLL into a specified process by its PID.
- **Verbose Output**: Support for verbose output to assist with debugging.
- **Help Command**: Easily access usage instructions.

## 📜 Usage

To run the DLL injector, compile the code and execute the resulting binary from the command line with the following syntax:

```bash
inject_dll.exe <pid> <dll path> [--verbose] [--help]

```
# 🧩 Arguments

```<pid>```: 🆔 The Process ID of the target application where the DLL will be injected.

```<dll path>``` : 📂 The full path to the DLL file you want to inject.

```--verbose``` : 🛠️ (Optional) Enable verbose output for additional debugging information.

```--help``` : ❓ (Optional) Show help message with usage instructions.

# 🔍 Example
```bash
inject_dll.exe 1234 C:\path\to\your.dll --verbose
```

# ✅ Requirements

Operating System: Windows
Compiler: A C++ compiler that supports the Windows API (e.g., Microsoft Visual Studio).

# 🛠️ Implementation Details

Process Handling: The application opens a target process with permissions to read, write, and create threads.
Memory Allocation: Allocates memory in the target process for the DLL path.
Write Process Memory: Writes the DLL path into the allocated memory.
Thread Creation: Creates a remote thread in the target process that executes LoadLibraryA with the DLL path as an argument.
Resource Management: Utilizes a class DataToFree to handle cleanup of allocated resources automatically.

# ⚠️ Error Handling
The application includes basic error handling that reports issues related to opening processes, memory allocation, writing to memory, and creating threads. Detailed error messages are provided to aid debugging.

# 📄 License

This project is licensed under the MIT License. See the LICENSE file for details.

# 🤝 Contributions

Contributions are welcome! Please fork the repository and submit a pull request for any enhancements or bug fixes. Let's make this project even better together! 🎉

📫 Contact
For any inquiries or feedback, feel free to reach out to the project maintainer.

Happy coding! 💻✨
