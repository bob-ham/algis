#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../offsets.hpp"


// lightweight string
struct RbxStr {
	char buf[512] = {};
	int len = 0;

	RbxStr() = default;

	RbxStr(const char* s) {
		if (!s) return;
		int i = 0;
		while (s[i] && i < 511) { buf[i] = s[i]; i++; };
		buf[i] = 0;
		len = i;
	}

	const char* c_str() const { return buf; }
	int size() const { return len; }
	bool empty() const { return len == 0; }

    bool operator==(const RbxStr& o) const {
        if (len != o.len) return false;
        for (int i = 0; i < len; i++)
            if (buf[i] != o.buf[i]) return false;
        return true;
    }

    bool operator==(const char* s) const {
        if (!s) return len == 0;
        int i = 0;
        while (s[i] && i < len) {
            if (buf[i] != s[i]) return false;
            i++;
        }
        return i == len && s[i] == 0;
    }
};
// lieghtweight vector
template<typename T>
struct RbxVec {
    T* data = nullptr;
    size_t count = 0;
    size_t capacity = 0;

    ~RbxVec() {
        if (data) HeapFree(GetProcessHeap(), 0, data);
    }

    void push_back(const T& val) {
        if (count >= capacity) {
            size_t new_cap = capacity ? capacity * 2 : 4;
            T* new_data = reinterpret_cast<T*>(
                HeapAlloc(GetProcessHeap(), 0, new_cap * sizeof(T)));
            if (!new_data) return;
            if (data) {
                memcpy(new_data, data, count * sizeof(T));
                HeapFree(GetProcessHeap(), 0, data);
            }
            data = new_data;
            capacity = new_cap;
        }
        data[count++] = val;
    }

    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }
    size_t size()  const { return count; }
    bool   empty() const { return count == 0; }
    T* begin() const { return data; }
    T* end()   const { return data + count; }
};


using FindFirstChild_t = uintptr_t(__fastcall*)(uintptr_t instance, const char* name, bool recursive);

namespace RBX {
    // logging functions
    namespace Log {
        inline HANDLE hFile = INVALID_HANDLE_VALUE;

        inline void Init(const wchar_t* path = L"C:\\Users\\Public\\rbxlog.log") {
            hFile = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        }

        inline void Print(const char* fmt, ...) {
            if (hFile == INVALID_HANDLE_VALUE) return;
            char buf[1024];
            va_list args;
            va_start(args, fmt);
            wvsprintfA(buf, fmt, args); 
            va_end(args);
            DWORD written = 0;
            int len = 0;
            while (buf[len]) len++;
            WriteFile(hFile, buf, len, &written, nullptr);
        }

        inline void Close() {
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
            }
        }
    } // namespace Log

    // memory functions
    namespace Memory {
        inline uintptr_t ModuleBase() {
            return (uintptr_t)GetModuleHandleA("RobloxPlayerBeta.exe");
        }
        template<typename T>
        inline T Read(uintptr_t addr) {
            if (!addr) return T{};
            __try {
                return *reinterpret_cast<T*>(addr);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return T{};
            }
        }

        template<typename T>
        inline void Write(uintptr_t addr, T val) {
            if (!addr) return;
            __try {
                *reinterpret_cast<T*>(addr) = val;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        static bool ReadStringRaw(uintptr_t addr, char* buf, size_t& outSize) {
            __try {
                size_t size = *reinterpret_cast<size_t*>(addr + 0x10);
                if (size == 0 || size > 512) return false;

                const char* data;
                if (size <= 15) {
                    data = reinterpret_cast<const char*>(addr);
                }
                else {
                    uintptr_t ptr = *reinterpret_cast<uintptr_t*>(addr);
                    if (!ptr) return false;
                    data = reinterpret_cast<const char*>(ptr);
                }

                memcpy(buf, data, size);
                outSize = size;
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }

        inline RbxStr ReadString(uintptr_t addr) {
            if (!addr) return RbxStr("");
            char buf[512]{};
            size_t size = 0;
            if (!ReadStringRaw(addr, buf, size)) return RbxStr("");
            buf[size] = 0;
            return RbxStr(buf);
        }



    } // namespace Memory

    class Instance {
    protected:
        uintptr_t Address;
    public:
        Instance(uintptr_t addr = 0) : Address(addr) {}

        uintptr_t GetAddress() const { return Address; }
        operator bool() const { return Address != 0; }

        RbxStr GetName() const {
            if (!Address) return RbxStr("");
            uintptr_t nmContainer = Memory::Read<uintptr_t>(Address + Offsets::Instance::Name);
            if (!nmContainer) return RbxStr("");
            return Memory::ReadString(nmContainer + Offsets::Instance::Name);
        }

        RbxVec<Instance> GetChildren() const {
            RbxVec<Instance> children;
            if (!Address) return children;

            uintptr_t start = Memory::Read<uintptr_t>(Address + Offsets::Instance::Children);
            uintptr_t end = Memory::Read<uintptr_t>(Address + Offsets::Instance::ChildrenCtrl);

            if (!start || !end || end <= start) return children;
            if (end - start > 0x10000) return children;

            for (uintptr_t ptr = start; ptr < end; ptr += 0x8) {
                uintptr_t child = Memory::Read<uintptr_t>(ptr);
                if (!child) continue;
                children.push_back(Instance(child));
            }
            return children;
        }

        Instance FindFirstChild(const char* name) const {
            if (!Address) return Instance(0);
            uintptr_t base = Memory::ModuleBase();
            auto fn = reinterpret_cast<FindFirstChild_t>(base + Offsets::Instance::FindFirstChild);
            return Instance(fn(Address, name, false));
        }
    };

    // SROBLOX
    struct SROBLOX {
        uintptr_t GetDM() {
            using GetDM_t = uintptr_t(__fastcall*)();
            uintptr_t base = Memory::ModuleBase();
            auto fn = reinterpret_cast<GetDM_t>(base + Offsets::DataModel::Get);
            return fn();
        }

        uintptr_t GetTS() {
            uintptr_t base = Memory::ModuleBase();
            return Memory::Read<uintptr_t>(base + Offsets::TaskScheduler::TaskScheduler);
        }
    }; // struct SROBLOX

    inline SROBLOX Fetch;

} // namespace RBX