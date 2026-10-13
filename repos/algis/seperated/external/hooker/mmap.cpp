#include "mmap.hpp"
#include "../../global/syscalls.hpp"

// manual mapper defs
mapper::mapper(Process& process) : process(process) {}

// fix relocs
bool mapper::fix_relocations(PVOID localBase, uintptr_t remoteBase) {
  auto dosHeader = (PIMAGE_DOS_HEADER)localBase;
  auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)localBase + dosHeader->e_lfanew);

  uintptr_t delta = remoteBase - ntHeaders->OptionalHeader.ImageBase;
  if (delta == 0)
    return true;

  auto relocDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
  if (relocDir->VirtualAddress == 0)
    return true;

  auto reloc = (PIMAGE_BASE_RELOCATION)((uintptr_t)localBase + relocDir->VirtualAddress);

  while (reloc->VirtualAddress) {
    DWORD entryCount = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
    auto entries = (PWORD)(reloc + 1);

    for (DWORD i = 0; i < entryCount; i++) {
      if ((entries[i] >> 12) == IMAGE_REL_BASED_DIR64) {
        uintptr_t* patch = (uintptr_t*)((uintptr_t)localBase + reloc->VirtualAddress + (entries[i] & 0xFFF));
        *patch += delta;
      }
    }
    reloc = (PIMAGE_BASE_RELOCATION)((uintptr_t)reloc + reloc->SizeOfBlock);
  }
  return true;
}

// resolve imports
bool mapper::resolve_imports(PVOID localBase) {
  auto dosHeader = (PIMAGE_DOS_HEADER)localBase;
  auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)localBase + dosHeader->e_lfanew);

  auto importDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (importDir->VirtualAddress == 0)
    return true;

  auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)localBase + importDir->VirtualAddress);

  while (importDesc->Name) {
    const char* moduleName = (const char*)((uintptr_t)localBase + importDesc->Name);
    HMODULE mod = LoadLibraryA(moduleName);
    if (!mod)
      return false;

    auto thunk = (PIMAGE_THUNK_DATA)((uintptr_t)localBase + importDesc->FirstThunk);
    auto origThunk = (PIMAGE_THUNK_DATA)((uintptr_t)localBase + importDesc->OriginalFirstThunk);

    while (thunk->u1.AddressOfData) {
      if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal)) {
        thunk->u1.Function = (uintptr_t)GetProcAddress(mod, (LPCSTR)IMAGE_ORDINAL(origThunk->u1.Ordinal));
      } else {
        auto importByName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)localBase + origThunk->u1.AddressOfData);
        thunk->u1.Function = (uintptr_t)GetProcAddress(mod, importByName->Name);
      }
      if (!thunk->u1.Function)
        return false;
      thunk++;
      origThunk++;
    }
    importDesc++;
  }
  return true;
}

// register seh
bool mapper::register_seh(PVOID localBase, uintptr_t remoteBase, SIZE_T imageSize) {
  auto dosHeader = (PIMAGE_DOS_HEADER)localBase;
  auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)localBase + dosHeader->e_lfanew);

  auto exceptionDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
  if (exceptionDir->VirtualAddress == 0)
    return true;

  auto funcTable = (PRUNTIME_FUNCTION)((uintptr_t)localBase + exceptionDir->VirtualAddress);
  DWORD entryCount = exceptionDir->Size / sizeof(RUNTIME_FUNCTION);

  return RtlAddFunctionTable(funcTable, entryCount, remoteBase);
}

mapper::map_result mapper::map(const std::wstring& dll_path) {
  map_result result = {0, 0, false};
  auto _NtCreateSection = Syscall::CreateSection;
  auto _NtMapViewOfSection = Syscall::MapViewOfSection;
  auto _NtUnmapViewOfSection = Syscall::UnmapViewOfSection;
  // check
  if (!Syscall::SSN::NtCreateSection.resolved ||
      !Syscall::SSN::NtMapViewOfSection.resolved ||
      !Syscall::SSN::NtUnmapViewOfSection.resolved)
    return result;

  // read dll from disk
  HANDLE file = CreateFileW(dll_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  if (file == INVALID_HANDLE_VALUE)
    return result;

  DWORD fileSize = GetFileSize(file, nullptr);
  std::vector<BYTE> dllBuffer(fileSize);
  DWORD bytesRead;
  ReadFile(file, dllBuffer.data(), fileSize, &bytesRead, nullptr);
  CloseHandle(file);

  auto dosHeader = (PIMAGE_DOS_HEADER)dllBuffer.data();
  auto ntHeaders = (PIMAGE_NT_HEADERS)(dllBuffer.data() + dosHeader->e_lfanew);
  SIZE_T imageSize = ntHeaders->OptionalHeader.SizeOfImage;

  // create section
  HANDLE section = nullptr;
  LARGE_INTEGER sectionSize;
  sectionSize.QuadPart = (LONGLONG)imageSize;
  NTSTATUS status = _NtCreateSection(&section, SECTION_ALL_ACCESS, nullptr, &sectionSize, PAGE_EXECUTE_READWRITE, SEC_COMMIT, nullptr);
  if (!NT_SUCCESS(status))
    return result;

  // map locally
  PVOID localBase = nullptr;
  SIZE_T viewSize = imageSize;
  status = _NtMapViewOfSection(section, GetCurrentProcess(), &localBase, 0, imageSize, nullptr, &viewSize, Syscall::ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
  if (!NT_SUCCESS(status)) {
    CloseHandle(section);
    return result;
  }

  // map remotely
  PVOID remoteBase = nullptr;
  viewSize = imageSize;
  status = _NtMapViewOfSection(section, process.get_handle(), &remoteBase, 0, imageSize, nullptr, &viewSize, Syscall::ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
  if (!NT_SUCCESS(status)) {
    _NtUnmapViewOfSection(GetCurrentProcess(), localBase);
    CloseHandle(section);
    return result;
  }

  // copy headers
  memcpy(localBase, dllBuffer.data(), ntHeaders->OptionalHeader.SizeOfHeaders);

  // copy sections
  auto section_header = IMAGE_FIRST_SECTION(ntHeaders);
  for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
    if (section_header[i].SizeOfRawData == 0)
      continue;
    PVOID dest = (PVOID)((uintptr_t)localBase + section_header[i].VirtualAddress);
    PVOID src = (PVOID)(dllBuffer.data() + section_header[i].PointerToRawData);
    memcpy(dest, src, section_header[i].SizeOfRawData);
  }

  // fix relocations and imports against remote base
  fix_relocations(localBase, (uintptr_t)remoteBase);
  resolve_imports(localBase);
  register_seh(localBase, (uintptr_t)remoteBase, imageSize);

  // get exported function RVA
  auto exportDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  if (exportDir->VirtualAddress != 0) {
    auto exportTable = (PIMAGE_EXPORT_DIRECTORY)((uintptr_t)localBase + exportDir->VirtualAddress);
    auto names = (DWORD*)((uintptr_t)localBase + exportTable->AddressOfNames);
    auto functions = (DWORD*)((uintptr_t)localBase + exportTable->AddressOfFunctions);
    auto ordinals = (WORD*)((uintptr_t)localBase + exportTable->AddressOfNameOrdinals);

    for (DWORD i = 0; i < exportTable->NumberOfNames; i++) {
      const char* name = (const char*)((uintptr_t)localBase + names[i]);
      // !! CHANGE THIS IF YOU WANT !!
      if (strcmp(name, "ThreadpoolCallback") == 0) {
        result.entry_point = (uintptr_t)remoteBase + functions[ordinals[i]];
        break;
      }
    }
  }

  result.remote_base = (uintptr_t)remoteBase;
  result.success = true;

  // unmap local view, remote stays mapped
  _NtUnmapViewOfSection(GetCurrentProcess(), localBase);

  return result;
}