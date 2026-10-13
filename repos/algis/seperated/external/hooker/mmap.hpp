#pragma once

#include "process.hpp"

// class mapper
class mapper {
private:
  Process& process;
  bool fix_relocations(PVOID localBase, uintptr_t remoteBase);
  bool resolve_imports(PVOID localBase);
  bool register_seh(PVOID localBase, uintptr_t remoteBase, SIZE_T imageSize);

public:
  mapper(Process& process);

  struct map_result {
    std::uintptr_t remote_base;
    std::uintptr_t entry_point;
    bool success;
  };

  map_result map(const std::wstring& dll_path);
};