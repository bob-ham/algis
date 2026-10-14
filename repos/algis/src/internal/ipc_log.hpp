#pragma once
#include "../global/ipc.hpp"


#if defined(ALGIS_INTERNAL)
inline ALGIS_API _ipc_log_(LPCWSTR ipc_name, std::uint8_t level, const char* msg) {
  if (!ipc_name || !msg)
    return AL_INVALID_PARAMETER;

  IpcHeader header = {};
  std::memcpy(header.magic, IPC_MAGIC, sizeof(header.magic));
  header.transport = XPIPE;

  IpcCmd cmd = {};
  cmd.cmd_id = AL_LOG;

  IpcLog log_payload = {};
  log_payload.level = level;
  log_payload.size = static_cast<std::uint32_t>(std::min(std::strlen(msg), (size_t)511));
  std::memcpy(log_payload.msg, msg, log_payload.size);

  size_t total = sizeof(IpcHeader) + sizeof(IpcCmd) + sizeof(IpcLog);
  std::vector<BYTE> packet(total);
  std::memcpy(packet.data(), &header, sizeof(IpcHeader));
  std::memcpy(packet.data() + sizeof(IpcHeader), &cmd, sizeof(IpcCmd));
  std::memcpy(packet.data() + sizeof(IpcHeader) + sizeof(IpcCmd), &log_payload, sizeof(IpcLog));

  HANDLE hPipe = INVALID_HANDLE_VALUE;
  ALGIS_STATUS status = _open_pipe_(ipc_name, 5000, &hPipe);
  if (status != ALGIS_OK)
    return status;

  DWORD bytes_written = 0;
  BOOL success = WriteFile(hPipe, packet.data(), static_cast<DWORD>(total), &bytes_written, NULL);
  _destroy_pipe_(&hPipe);

  return (success && bytes_written == total) ? ALGIS_OK : AL_WRITE_FAILED;
}
#endif