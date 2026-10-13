#include "dispatch.hpp"

#if defined(ALGIS_INTERNAL)

dispatch::dispatch(LPCWSTR pipe_name)
    : pipe_name(pipe_name), running(false) {}

dispatch::~dispatch() {
  stop();
}

void dispatch::start() {
  this->running = true;
  this->thread = std::thread(&dispatch::loop, this);
}

void dispatch::stop() {
  running = false;
  if (this->thread.joinable()) {
    this->thread.join();
  }
}

void dispatch::loop() {
  while (running) {
    HANDLE hPipe = INVALID_HANDLE_VALUE;

    ALGIS_STATUS status = _create_pipe_(pipe_name, 4096, &hPipe);
    if (status != ALGIS_OK)
      continue;

    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (!connected) {
      _destroy_pipe_(&hPipe);
      continue;
    }

    std::vector<BYTE> buf(sizeof(IpcHeader) + sizeof(IpcCmd) + 512);
    DWORD bytes_read = 0;

    BOOL sucess = ReadFile(hPipe, buf.data(), static_cast<DWORD>(buf.size()), &bytes_read, NULL);

    if (!sucess || bytes_read < sizeof(IpcHeader) + sizeof(IpcCmd)) {
      _destroy_pipe_(&hPipe);
      continue;
    }

    IpcHeader* header = reinterpret_cast<IpcHeader*>(buf.data());
    if (std::memcmp(header->magic, IPC_MAGIC, sizeof(header->magic)) != 0) {
      _destroy_pipe_(&hPipe);
      continue;
    }

    BYTE* payload = buf.data() + sizeof(IpcHeader);
    handle(hPipe, payload, bytes_read - static_cast<DWORD>(sizeof(IpcHeader)));

    _destroy_pipe_(&hPipe);
  }
}

void dispatch::on_data(AlgisDataCallback callback) {
  data_callback = std::move(callback);
}

void dispatch::handle(HANDLE pipe, const BYTE* buf, DWORD size) {
  if (size < sizeof(IpcCmd))
    return;

  const IpcCmd* cmd = reinterpret_cast<const IpcCmd*>(buf);

  IpcReserve reserve = {};
  reserve.cmd_id = cmd->cmd_id;
  reserve.status = ALGIS_OK;
  reserve.size = 0;

  switch (cmd->cmd_id) {
  case AL_PING: {
    // !! stub
    break;
  }

  case AL_DATA: {
    if (data_callback) {
      const BYTE* payload = buf + sizeof(IpcCmd);
      DWORD payload_size = size - static_cast<DWORD>(sizeof(IpcCmd));
      data_callback(payload, payload_size);
    }
    // ? usage: ?
    /*
     * dispatch d([PIPE_NAME]);
     * d.on_data([](const BYTE* buf, DWORD size) {
     *  handle incoming data
     * });
     * d.start();
     */
    break;
  }

  case AL_EXECUTE: { /* !! stub (example: reserve.status = user_on_execute(cmd->addr);  !! */
    break;
  }

  default: {
    reserve.status = AL_INVALID_PARAMETER;
    break;
  }
  }
  DWORD written = 0;
  WriteFile(pipe, &reserve, sizeof(IpcReserve), &written, NULL);
}

#endif