// ipc.hpp 
#pragma once
#include "../algis.h"

// change; added IPC_MAGIC as a constexpr as its an internal macro that doesnt need to be exposed to the public API
static constexpr char IPC_MAGIC[] = "algis_ipc";


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
inline ALGIS_API _xsend_(int transport_type, LPCWSTR ipc_name, const void* data, size_t data_size) {
    if (!ipc_name || !data || data_size == 0)
        return AL_INVALID_PARAMETER;

    if (transport_type == XPIPE) {
        HANDLE hPipe = INVALID_HANDLE_VALUE;

        ALGIS_STATUS status = _open_pipe_(ipc_name, 5000, &hPipe);
        if (status != ALGIS_OK)
            return status;

        IpcHeader header = {};
        std::memcpy(header.magic, IPC_MAGIC, sizeof(header.magic));
        header.transport     = XPIPE;
        header.payload_size  = static_cast<std::uint32_t>(data_size);

        std::vector<BYTE> packet(sizeof(IpcHeader) + data_size);
        std::memcpy(packet.data(), &header, sizeof(IpcHeader));
        std::memcpy(packet.data() + sizeof(IpcHeader), data, data_size);

        DWORD written = 0;
        BOOL ok = WriteFile(hPipe, packet.data(), static_cast<DWORD>(packet.size()), &written, NULL);
        _destroy_pipe_(&hPipe);

        if (!ok || written != static_cast<DWORD>(packet.size()))
            return AL_WRITE_FAILED;

        return ALGIS_OK;
    }

    else if (transport_type == XSMEM) {
        wchar_t event_data[256], event_ack[256];
        swprintf_s(event_data, L"%s_DATA", ipc_name);
        swprintf_s(event_ack,  L"%s_ACK",  ipc_name);

        HANDLE hMap    = NULL;
        void*  pBuffer = NULL;

        ALGIS_STATUS status = _create_smem_(ipc_name, static_cast<int>(data_size), &hMap, &pBuffer);
        if (status != ALGIS_OK)
            return status;

        std::memcpy(pBuffer, data, data_size);

        HANDLE hData = OpenEventW(EVENT_MODIFY_STATE, FALSE, event_data);
        if (!hData) {
            _destroy_smem_(hMap, pBuffer);
            return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
        }
        SetEvent(hData);
        CloseHandle(hData);

        HANDLE hAck = OpenEventW(SYNCHRONIZE, FALSE, event_ack);
        if (hAck) {
            WaitForSingleObject(hAck, 5000);
            CloseHandle(hAck);
        }

        _destroy_smem_(hMap, pBuffer);
        return ALGIS_OK;
    }

    return AL_INVALID_PARAMETER;
}

inline ALGIS_API _xrecieve_(int transport_type, LPCWSTR ipc_name, void* buffer, size_t buffer_size, DWORD* bytes_read, DWORD ms) {
    if (!ipc_name || !buffer || buffer_size == 0 || !bytes_read)
        return AL_INVALID_PARAMETER;

    *bytes_read = 0;

    if (transport_type == XPIPE) {
        HANDLE hPipe = INVALID_HANDLE_VALUE;

        DWORD pipe_buf = static_cast<DWORD>(sizeof(IpcHeader) + buffer_size);
        ALGIS_STATUS status = _create_pipe_(ipc_name, pipe_buf, &hPipe);
        if (status != ALGIS_OK)
            return status;

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!connected) {
            _destroy_pipe_(&hPipe);
            return AL_CONNECT_FAILED;
        }

        std::vector<BYTE> read_buffer(pipe_buf);
        DWORD dwRead = 0;
        BOOL ok = ReadFile(hPipe, read_buffer.data(), static_cast<DWORD>(read_buffer.size()), &dwRead, NULL);
        _destroy_pipe_(&hPipe);

        if (!ok || dwRead < sizeof(IpcHeader))
            return AL_READ_FAILED;

        IpcHeader* header = reinterpret_cast<IpcHeader*>(read_buffer.data());
        if (std::memcmp(header->magic, IPC_MAGIC, sizeof(header->magic)) != 0)
            return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

        DWORD payload_size = dwRead - static_cast<DWORD>(sizeof(IpcHeader));
        if (payload_size > static_cast<DWORD>(buffer_size))
            payload_size = static_cast<DWORD>(buffer_size);

        std::memcpy(buffer, read_buffer.data() + sizeof(IpcHeader), payload_size);
        *bytes_read = payload_size;
        return ALGIS_OK;
    }

    else if (transport_type == XSMEM) {
        wchar_t event_data[256], event_ack[256];
        swprintf_s(event_data, L"%s_DATA", ipc_name);
        swprintf_s(event_ack,  L"%s_ACK",  ipc_name);

        HANDLE hData = CreateEventW(NULL, FALSE, FALSE, event_data);
        HANDLE hAck  = CreateEventW(NULL, FALSE, FALSE, event_ack);

        if (!hData || !hAck) {
            if (hData) CloseHandle(hData);
            if (hAck)  CloseHandle(hAck);
            return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
        }

        DWORD wait = WaitForSingleObject(hData, ms);
        if (wait != WAIT_OBJECT_0) {
            CloseHandle(hData);
            CloseHandle(hAck);
            return (wait == WAIT_TIMEOUT) ? AL_WAIT_TIMEOUT : AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
        }

        HANDLE hMap    = NULL;
        void*  pBuffer = NULL;

        ALGIS_STATUS status = _open_smem_(ipc_name, static_cast<int>(buffer_size), &hMap, &pBuffer);
        if (status != ALGIS_OK) {
            SetEvent(hAck); 
            CloseHandle(hData);
            CloseHandle(hAck);
            return status;
        }

        std::memcpy(buffer, pBuffer, buffer_size);
        *bytes_read = static_cast<DWORD>(buffer_size);

        _destroy_smem_(hMap, pBuffer);

        SetEvent(hAck);
        CloseHandle(hData);
        CloseHandle(hAck);
        return ALGIS_OK;
    }

    return AL_INVALID_PARAMETER;
}


// dispatcher stuff will be added later 