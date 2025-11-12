#include "SReg.h"
#include <storm/String.hpp>

#include <toml++/toml.hpp>
#include <string>
#include <algorithm>

namespace {
  const char* SRegGetBaseKeyInternal(DWORD flags) {
    return (flags & SREG_FLAG_BATTLENET) ? "Software\\Battle.net\\" : "Software\\Blizzard Entertainment\\";
  }

  toml::table GetOrCreateTable(toml::node_view<toml::node>& node) {
    return node.is_table() ? *node.as_table() : toml::table{};
  }

  toml::table GetTomlSettings(DWORD flags) {
    std::string path = SRegGetBaseKeyInternal(flags);
    std::replace(std::begin(path), std::end(path), '\\', '/');

    try {
      return toml::parse_file(path + "settings.toml");
    }
    catch (...) {
      return toml::parse("");
    }
  }

  void WriteTomlSettings(toml::table& settings, DWORD flags) {
    std::string path = SRegGetBaseKeyInternal(flags);
    std::replace(std::begin(path), std::end(path), '\\', '/');

    try {
      auto& f = std::ofstream(path + "settings.toml");
      f << settings << "\n";
    }
    catch (...) {}
  }
}

// @421
BOOL STORMAPI SRegLoadData(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPVOID buffer, DWORD buffersize, DWORD* bytesread) {
  auto& settings = GetTomlSettings(flags);

  if (!keyname || !valuename) return FALSE;
  if (bytesread) *bytesread = 0;

  DWORD read = 0;
  auto node = settings[keyname][valuename];
  if (!node.is_array()) return FALSE;

  toml::array arraynode = *node.as_array();
  arraynode.flatten();

  if (flags & SREG_FLAG_MULTISZ) {
    char* pBuffer = reinterpret_cast<char*>(buffer);

    for (auto&& elem : arraynode) {
      if (!elem.is_string()) return FALSE;

      std::string elemstr = elem.as_string()->get();

      if (pBuffer) {
        if (read >= buffersize) return FALSE;
        
        SStrCopy(&pBuffer[read], elemstr.c_str(), buffersize - read);
      }
      read += static_cast<DWORD>(elemstr.size()) + 1;
    }
  }
  else {
    std::uint8_t* pBuffer = reinterpret_cast<std::uint8_t*>(buffer);

    for (auto&& elem : arraynode) {
      if (!elem.is_integer()) return FALSE;
      if (pBuffer) {
        if (read >= buffersize) return FALSE;

        pBuffer[read] = static_cast<std::uint8_t>(elem.as_integer()->get());
      }
      read++;
    }
  }
  if (bytesread) *bytesread = read;
  return TRUE;
}

// @422
BOOL STORMAPI SRegLoadString(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPSTR buffer, DWORD bufferchars) {
  auto& settings = GetTomlSettings(flags);

  if (!keyname || !valuename || !buffer || bufferchars == 0) return FALSE;

  auto& node = settings[keyname][valuename];

  std::optional<std::string> result = std::nullopt;
  if (node.is_integer()) {
    result = std::to_string(node.as_integer()->get());
  }
  else if (node.is_string()) {
    result = node.as_string()->get();
  }

  if (!result) return FALSE;

  SStrCopy(buffer, result->c_str(), bufferchars);
  return TRUE;
}

// @423
BOOL STORMAPI SRegLoadValue(LPCSTR keyname, LPCSTR valuename, DWORD flags, DWORD* value) {
  auto& settings = GetTomlSettings(flags);

  if (!keyname || !valuename || !value) return FALSE;

  auto& node = settings[keyname][valuename];

  if (node.is_integer()) {
    *value = static_cast<DWORD>(node.as_integer()->get());
    return TRUE;
  }
  else if (node.is_string()) {
    *value = std::stoul(node.as_string()->get());
    return TRUE;
  }
  return FALSE;
}

// @424
BOOL STORMAPI SRegSaveData(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPVOID data, DWORD databytes) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);

  if (flags & SREG_FLAG_MULTISZ) {
    char* psz = reinterpret_cast<char*>(data);
    DWORD written = 0;

    toml::array arr;
    while (written < databytes) {
      std::string str(&psz[written]);
      arr.push_back(str);
      
      written += static_cast<DWORD>(str.size()) + 1;
    }

    tbl.insert_or_assign(valuename, arr);
  }
  else {
    std::uint8_t* bytes = reinterpret_cast<std::uint8_t*>(data);
    toml::array arr;
    for (unsigned i = 0; i < databytes; i++) {
      arr.push_back(bytes[i]);
    }
    tbl.insert_or_assign(valuename, arr);
  }

  settings.insert_or_assign(keyname, tbl);
  WriteTomlSettings(settings, flags);
  return TRUE;
}

// @425
BOOL STORMAPI SRegSaveString(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPCSTR string) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);
  tbl.insert_or_assign(valuename, string);

  settings.insert_or_assign(keyname, tbl);
  WriteTomlSettings(settings, flags);
  return TRUE;
}

// @426
BOOL STORMAPI SRegSaveValue(LPCSTR keyname, LPCSTR valuename, DWORD flags, DWORD value) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);
  tbl.insert_or_assign(valuename, value);

  settings.insert_or_assign(keyname, tbl);
  WriteTomlSettings(settings, flags);
  return TRUE;
}

// @427
BOOL STORMAPI SRegGetBaseKey(DWORD flags, LPSTR buffer, DWORD buffersize) {
  if (buffer == nullptr || buffersize == 0) return FALSE;

  SStrCopy(buffer, SRegGetBaseKeyInternal(flags), buffersize);
  return TRUE;
}

// @428
BOOL STORMAPI SRegDeleteValue(LPCSTR keyname, LPCSTR valuename, DWORD flags) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);
  tbl.erase(valuename);

  settings.insert_or_assign(keyname, tbl);
  WriteTomlSettings(settings, flags);
  return TRUE;
}

// @429
BOOL STORMAPI SRegEnumKey(LPCSTR keyname, DWORD flags, DWORD index, LPSTR buffer, DWORD bufferchars) {
  auto& settings = GetTomlSettings(flags);
  
  toml::table tbl = GetOrCreateTable(settings[keyname]);
  if (index >= tbl.size()) return FALSE;

  auto it = std::cbegin(tbl);
  std::advance(it, index);

  auto key = std::string(it->first.str());
  SStrCopy(buffer, key.c_str(), bufferchars);

  return TRUE;
}

// @430
BOOL STORMAPI SRegGetNumSubKeys(LPCSTR keyname, DWORD flags, DWORD* subkeys) {
  auto& settings = GetTomlSettings(flags);
  
  toml::table tbl = GetOrCreateTable(settings[keyname]);
  if (subkeys) *subkeys = static_cast<DWORD>(tbl.size());
  return TRUE;
}

