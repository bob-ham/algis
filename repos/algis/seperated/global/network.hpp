#pragma once
#include "../algis.h"

#if !defined(ALGIS_LEAN)

static std::map<std::string, std::uintptr_t> offset_map;

inline std::string _fetch_url_(const std::string& url) {
  HINTERNET session = InternetOpenA("algis", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
  if (!session)
    return {};

  HINTERNET connection = InternetOpenUrlA(session, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
  if (!connection) {
    InternetCloseHandle(session);
    return {};
  }

  std::string result;
  char buf[4096];
  DWORD bytes_read = 0;

  while (InternetReadFile(connection, buf, sizeof(buf), &bytes_read) && bytes_read > 0) {
    result.append(buf, bytes_read);
  }
  InternetCloseHandle(connection);
  InternetCloseHandle(session);
  return result;
}

inline void _parse_offsets_(const std::string& src) {
  std::regex pattern(R"(constexpr\s+\S+\s+(\w+)\s*=\s*(0x[0-9a-fA-F]+|\d+)\s*;)");
  auto begin = std::sregex_iterator(src.begin(), src.end(), pattern);
  auto end = std::sregex_iterator{};

  for (auto it = begin; it != end; it++) {
    offset_map[(*it)[1].str()] = static_cast<std::uintptr_t>(
      std::stoull((*it)[2].str(), nullptr, 0));
  }
}

inline ALGIS_API _fetch_(const std::string& url, const std::string& save_path) {
  std::string src = _fetch_url_(url);
  if (src.empty())
    return AL_READ_FAILED;

  std::ofstream file(save_path, std::ios::out | std::ios::trunc);
  if (!file.is_open())
    return AL_FILE_NOT_FOUND;

  file << src;
  file.close();

  _parse_offsets_(src);
  return ALGIS_OK;
}

inline std::uintptr_t _get_(const std::string& name) {
  auto it = offset_map.find(name);
  return it != offset_map.end() ? it->second : 0;
}

#endif