#pragma once
#include "../global/ipc.hpp"

#if defined(ALGIS_INTERNAL)

using AlgisDataCallback = std::function<void(const BYTE*, DWORD)>;

class dispatch {
public:
  dispatch(LPCWSTR pipe_name);
  ~dispatch();

  void start();
  void stop();
  void on_data(AlgisDataCallback callback);

private:
  LPCWSTR pipe_name;
  std::thread thread;
  bool running;
  AlgisDataCallback data_callback;

  void loop();
  void handle(HANDLE pipe, const BYTE* buf, DWORD size);
};

#endif