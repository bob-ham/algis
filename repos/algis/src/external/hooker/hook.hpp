// hook.hpp
#pragma once
#include "process.hpp"
#include "mmap.hpp"

#include <map>

struct Hook {
  void** target_slot;
  void* orig;
  // hook
  bool hook(Process& proc, void* jmpInstruction, void* newTarget) {
    /*
     * Calculate the address of the pointer slot.
     * jmp [rip + offset] - offset is at byte 3. Pointer is at instruction + 7 + offset.
     */
    int32_t offset = 0;
    SIZE_T read;
    proc.read_memr((PVOID)((uintptr_t)jmpInstruction + 3), &offset, sizeof(int32_t), &read);
    target_slot = (void**)((uintptr_t)jmpInstruction + 7 + offset);

    /* Read original memory. */
    proc.read_memr(target_slot, &orig, sizeof(void*), &read);

    /* Write the new target. */
    ULONG oldProt;
    proc.change_protection(target_slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
    SIZE_T written;
    proc.write_memr(target_slot, &newTarget, sizeof(void*), &written);
    proc.change_protection(target_slot, sizeof(void*), oldProt, &oldProt);

    /* We are hooked! */
    return true;
  }
  /* Unhook. */
  bool unhook(Process& proc) {
    ULONG oldProt;
    proc.change_protection(target_slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
    SIZE_T written;
    proc.write_memr(target_slot, &orig, sizeof(void*), &written);
    proc.change_protection(target_slot, sizeof(void*), oldProt, &oldProt);
    /* We are unhooked! */
    return true;
  }

};

// HookLib
struct HookLib {
  Process* process = nullptr;
  std::map<void*, Hook*> registry;
  mapper::map_result last_map_result = {0, 0, false};
};

inline HookLib& GetHookLib() {
  static HookLib lib;
  return lib;
}
