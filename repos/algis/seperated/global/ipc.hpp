// ipc.hpp 
#pragma once
#include "../algis.h"


// global code so no need for header guards 

namespace {
    inline bool IsValidIPCName(LPCWSTR name) {
        return name != nullptr && name[0] != L'\0';
    }
}


// server
inline ALGIS_API _create_pipe_(LPCWSTR pipe_name, DWORD buffer_size, HANDLE* o_handle) {
    if (!IsValidIPCName(pipe_name) || !o_handle) {
        return AL_INVALID_PARAMETER;
    }

    HANDLE hPipe = CreateNamedPipeW(
        pipe_name,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        buffer_size, buffer_size,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        return AL_CREATE_FAILED;
    }

    *o_handle = hPipe;
    return ALGIS_OK;
}

// client
inline ALGIS_API _open_pipe_(LPCWSTR pipe_name, DWORD ms, HANDLE *o_handle) {
    if (!IsValidIPCName(pipe_name) || !o_handle) return AL_INVALID_PARAMETER;
    
    *o_handle = INVALID_HANDLE_VALUE;

    if (!WaitNamedPipeW(pipe_name, ms)) {
        return AL_TIMEOUT;
    }

    HANDLE hPipe = CreateFileW(
        pipe_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        return AL_CONNECT_FAILED;
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL)) {
        CloseHandle(hPipe);
        return AL_MODE_CHANGE_FAILED;
    }

    *o_handle = hPipe;
    return ALGIS_OK;
}

// server
inline ALGIS_API _destroy_pipe_(HANDLE* pipe_handle) {
    if (!pipe_handle || *pipe_handle == INVALID_HANDLE_VALUE || *pipe_handle == NULL) {
        return AL_INVALID_PARAMETER;
    }

    HANDLE hPipe = *pipe_handle;
    *pipe_handle = INVALID_HANDLE_VALUE;

    DisconnectNamedPipe(hPipe);

    if (!CloseHandle(hPipe)) {
        return AL_BROKEN;
    }

    return ALGIS_OK;
}

// server
inline ALGIS_API _create_smem_(LPCWSTR mem_name, int buffer_size, HANDLE* o_handle, void** o_buffer) {
    if (!mem_name || buffer_size <= 0 || !o_handle || !o_buffer) {
        return AL_INVALID_PARAMETER;
    }

    *o_handle = NULL;
    *o_buffer = NULL;

    HANDLE hMap = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        static_cast<DWORD>(buffer_size),
        mem_name
    );

    if (hMap == NULL) {
        return AL_MAPPING_CREATE_FAILED;
    }

    void* pBuf = MapViewOfFile(
        hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        buffer_size
    );

    *o_handle = hMap;
    *o_buffer = pBuf;
    return ALGIS_OK;
}

// client
inline ALGIS_API _open_smem_(LPCWSTR mem_name, int buffer_size, HANDLE* o_handle, void** o_buffer) {
    if (!mem_name || buffer_size <= 0 || !o_handle || !o_buffer) {
        return AL_INVALID_PARAMETER;
    }

    *o_buffer = NULL;
    *o_handle = NULL;

    HANDLE hMap = OpenFileMappingW(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        mem_name
    );

    if (hMap == NULL) {
        return AL_MAPPING_OPEN_FAILED;
    }

    void* pBuf = MapViewOfFile(
        hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        buffer_size
    );

    if (pBuf == NULL) {
        return AL_MAP_VIEW_FAILED;
    }

    *o_buffer = pBuf;
    *o_handle = hMap;
    return ALGIS_OK;
}

inline ALGIS_API _destroy_smem_(HANDLE o_handle, void* buffer) {
    if (!o_handle || o_handle == INVALID_HANDLE_VALUE || !buffer) {
        return AL_INVALID_PARAMETER;
    }

    BOOL unmapped = TRUE;
    BOOL closed = TRUE;

    if (buffer) {
        unmapped = UnmapViewOfFile(buffer);
    }

    if (o_handle && o_handle != INVALID_HANDLE_VALUE) {
        closed = CloseHandle(o_handle);
    }

    if (!unmapped) return AL_UNMAP_VIEW_FAILED;
    if (!closed) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
    
    return ALGIS_OK;
}

// XSend and XRecieve
inline ALGIS_API _xsend_(int transport_type, LPCWSTR ipc_name, const void *data, size_t data_size) {
    if (!ipc_name || !data || data_size == 0) {
        return AL_INVALID_PARAMETER;
    }

    // named pipes
    if (transport_type == XPIPE) {
        HANDLE hPipe = INVALID_HANDLE_VALUE;

        ALGIS_STATUS status = _open_pipe_(ipc_name, 5000, &hPipe);
        if (status != ALGIS_OK) {
            return status;
        }

        IpcHeader header = {};
        std::memcpy(header.magic, IPC_MAGIC, sizeof(header.magic));
        header.payload_size = static_cast<std::uint32_t>(data_size);

        std::vector<BYTE> packet_buffer(sizeof(IpcHeader) + data_size);
        std::memcpy(packet_buffer.data(), &header, sizeof(IpcHeader));
        std::memcpy(packet_buffer.data() + sizeof(IpcHeader), data, data_size);

        DWORD bytes_written = 0;
        BOOL success = WriteFile(
            hPipe,
            packet_buffer.data(),
            static_cast<DWORD>(packet_buffer.size()),
            &bytes_written,
            NULL
        );

        _destroy_pipe_(&hPipe);

        if (!success || bytes_written != packet_buffer.size()) {
            return AL_WRITE_FAILED;
        }
        return ALGIS_OK;
    }

    // shared mem (NO IPC_MAGIC YET)
    else if (transport_type == XSMEM) {
        HANDLE hMap = NULL;
        void* pBuffer = NULL;

        ALGIS_STATUS status = _create_smem_(ipc_name, static_cast<int>(data_size), &hMap, &pBuffer);
        if (status != ALGIS_OK) {
            return status;
        }

        std::memcpy(pBuffer, data, data_size);
        
        wchar_t event_name[256];
        swprintf_s(event_name, L"%s_EVENT", ipc_name);

        HANDLE hEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, event_name);
        
        if (hEvent) {
            SetEvent(hEvent);
            CloseHandle(hEvent);
        }

        _destroy_smem_(hMap, pBuffer);
        return ALGIS_OK;
    }

    return AL_INVALID_PARAMETER;
}

inline ALGIS_API _xrecieve_(int transport_type, LPCWSTR ipc_name, void *buffer, size_t buffer_size, DWORD *bytes_read, DWORD ms) {
    if (!ipc_name || !buffer || buffer_size == 0 || !bytes_read) {
        return AL_INVALID_PARAMETER;
    }

    *bytes_read = 0;

    // named pipes
    if (transport_type == XPIPE) {
        HANDLE hPipe = INVALID_HANDLE_VALUE;

        size_t total_expected_size = sizeof(IpcHeader) + buffer_size;
        ALGIS_STATUS status = _create_pipe_(ipc_name, static_cast<DWORD>(buffer_size), &hPipe);
        if (status != ALGIS_OK) return status;

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (!connected) {
            _destroy_pipe_(&hPipe);
            return AL_CONNECT_FAILED;
        }

        std::vector<BYTE> read_buffer(total_expected_size);
        DWORD dwRead = 0;
        BOOL success = ReadFile(hPipe, read_buffer.data(), static_cast<DWORD>(read_buffer.size()), &dwRead, NULL);

        _destroy_pipe_(&hPipe);

        if (!success || dwRead < sizeof(IpcHeader)) {
            return AL_READ_FAILED;
        }

        IpcHeader* header = reinterpret_cast<IpcHeader*>(read_buffer.data());
        if (std::memcmp(header->magic, IPC_MAGIC, sizeof(header->magic)) != 0) {
            return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
        }

        DWORD acutal_payload_size = dwRead - sizeof(IpcHeader);
        if (acutal_payload_size > buffer_size) {
            acutal_payload_size = static_cast<DWORD>(buffer_size);
        }

        std::memcpy(buffer, read_buffer.data() + sizeof(IpcHeader), acutal_payload_size);
        *bytes_read = acutal_payload_size;
        return ALGIS_OK;
    }

    else if (transport_type == XSMEM) {
        wchar_t event_name[256];
        swprintf_s(event_name, L"%s_EVENT", ipc_name);

        HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, event_name);
        if (!hEvent) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

        DWORD wait_result = WaitForSingleObject(hEvent, ms);
        if (wait_result == WAIT_TIMEOUT) {
            CloseHandle(hEvent);
            return AL_WAIT_TIMEOUT;
        }
        else if (wait_result != WAIT_OBJECT_0) {
            CloseHandle(hEvent);
            return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
        }

        HANDLE hMap = NULL;
        void* pBuffer = NULL;

        ALGIS_STATUS status = _create_smem_(ipc_name, static_cast<int>(buffer_size), &hMap, &pBuffer);

        if (status != ALGIS_OK) {
            CloseHandle(hEvent);
            return status;
        }

        std::memcpy(buffer, pBuffer, buffer_size);
        *bytes_read = static_cast<DWORD>(buffer_size);

        _destroy_smem_(hMap, pBuffer);
        CloseHandle(hEvent);

        return ALGIS_OK;
    }

    return AL_INVALID_PARAMETER;
}


// dispatcher stuff will be added later 