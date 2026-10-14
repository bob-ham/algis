#define ALGIS_EXTERNAL
#include "../src/algis.h"

// example_ipc.cpp
// an example of how xsend and xrecieve work

struct data {
  float x, y, z;
  int health;
  char name[32];
};

// named pipes

// sender
void pipe_sender() {
  data d = {39.0f, 323.0f, 323.2f, 32, "test"};

  ALGIS_STATUS status = algis::xsend(
    XPIPE,
    L"\\\\.\\pipe\\algis_pipe",
    &d,
    sizeof(data));

  if (status != ALGIS_OK) {
    // log that it failed
  }
}

void pipe_recieve() {
  data d = {};
  DWORD bytes_read = 0;

  ALGIS_STATUS status = algis::xrecieve(
    XPIPE,
    L"\\\\.\\pipe\algis_pipe",
    &d,
    sizeof(data),
    &bytes_read,
    3000);

  if (status != ALGIS_OK) {
    // log that it failed
  }
  // do whatever you want with it
}

// shared memory
void smem_sender() {
  data d = {39.0f, 323.0f, 323.2f, 32, "test"};

  ALGIS_STATUS status = algis::xsend(
    XSMEM,
    L"Local\algis_mem",
    &d,
    sizeof(data));

  if (status != ALGIS_OK) {
    // log that it failed
  }
}

void smem_recieve() {
  data d = {};
  DWORD bytes_read = 0;

  ALGIS_STATUS status = algis::xrecieve(
    XSMEM,
    L"Local\algis_mem",
    &d,
    sizeof(data),
    &bytes_read,
    INFINITE);

  if (status != ALGIS_OK) {
    // log that it failed
  }
  // do whatever you want with it
}
