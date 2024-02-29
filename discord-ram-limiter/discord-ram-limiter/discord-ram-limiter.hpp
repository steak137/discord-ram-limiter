#pragma once
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

DWORD get_procress_id(const std::wstring& processName) {
    DWORD processIds[1024], bytesReturned;
    if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned))
        return 0;

    DWORD count = bytesReturned / sizeof(DWORD);
    for (DWORD i = 0; i < count; ++i) {
        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
        if (processHandle != NULL) {
            TCHAR buffer[MAX_PATH];
            if (GetModuleBaseName(processHandle, NULL, buffer, MAX_PATH) > 0) {
                std::wstring moduleName(buffer);
                if (moduleName == processName) {
                    CloseHandle(processHandle);
                    return processIds[i];
                }
            }
            CloseHandle(processHandle);
        }
    }
    return 0;
}

bool set_working_size(DWORD processId) {
    HANDLE processHandle = OpenProcess(PROCESS_SET_QUOTA, FALSE, processId);
    if (processHandle != NULL) {
        if (SetProcessWorkingSetSize(processHandle, (SIZE_T)-1, (SIZE_T)-1)) {
            CloseHandle(processHandle);
            return true;
        }
        CloseHandle(processHandle);
    }
    return false;
}

void monitor_and_limit_mem() {
    while (true) {
        DWORD processId = get_procress_id(L"Discord.exe");
        if (processId != 0) {
            PROCESS_MEMORY_COUNTERS pmc;
            HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            if (processHandle != NULL) {
                if (GetProcessMemoryInfo(processHandle, &pmc, sizeof(pmc))) {
                    if (pmc.WorkingSetSize > 500 * 1024 * 1024) { // 500 MB
                        std::cout << "Memory usage exceeded limit. Limiting memory usage for process Discord" << std::endl;
                        set_working_size(processId);
                    }
                }
                CloseHandle(processHandle);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}