#include "SReg.h"
#include <storm/String.hpp>

#include <toml++/toml.hpp>
#include <string>
#include <algorithm>

namespace {
  const char* SRegGetBaseKeyInternal(std::uint32_t flags) {
    return (flags & SREG_FLAG_BATTLENET) ? "Software\\Battle.net\\" : "Software\\Blizzard Entertainment\\";
  }

  toml::table GetOrCreateTable(toml::node_view<toml::node>& node) {
    return node.is_table() ? *node.as_table() : toml::table{};
  }

  toml::table GetTomlSettings(std::uint32_t flags) {
    std::string path = SRegGetBaseKeyInternal(flags);
    std::replace(std::begin(path), std::end(path), '\\', '/');

    try {
      return toml::parse_file(path + "settings.toml");
    }
    catch (...) {
      return toml::parse("");
    }
  }

  void WriteTomlSettings(toml::table& settings, std::uint32_t flags) {
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
BOOL STORMAPI SRegLoadData(const char* keyname, const char* valuename, std::uint32_t flags, void* buffer, std::uint32_t buffersize, std::uint32_t* bytesread) {
  auto& settings = GetTomlSettings(flags);

  if (!keyname || !valuename) return FALSE;
  if (bytesread) *bytesread = 0;

  std::uint32_t read = 0;
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
      read += static_cast<std::uint32_t>(elemstr.size()) + 1;
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
BOOL STORMAPI SRegLoadString(const char* keyname, const char* valuename, std::uint32_t flags, char* buffer, std::uint32_t bufferchars) {
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
BOOL STORMAPI SRegLoadValue(const char* keyname, const char* valuename, std::uint32_t flags, std::uint32_t* value) {
  auto& settings = GetTomlSettings(flags);

  if (!keyname || !valuename || !value) return FALSE;

  auto& node = settings[keyname][valuename];

  if (node.is_integer()) {
    *value = static_cast<std::uint32_t>(node.as_integer()->get());
    return TRUE;
  }
  else if (node.is_string()) {
    *value = std::stoul(node.as_string()->get());
    return TRUE;
  }
  return FALSE;
}

// @424
BOOL STORMAPI SRegSaveData(const char* keyname, const char* valuename, std::uint32_t flags, void* data, std::uint32_t databytes) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);

  if (flags & SREG_FLAG_MULTISZ) {
    char* psz = reinterpret_cast<char*>(data);
    std::uint32_t written = 0;

    toml::array arr;
    while (written < databytes) {
      std::string str(&psz[written]);
      arr.push_back(str);
      
      written += static_cast<std::uint32_t>(str.size()) + 1;
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
BOOL STORMAPI SRegSaveString(const char* keyname, const char* valuename, std::uint32_t flags, const char* string) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);
  tbl.insert_or_assign(valuename, string);

  settings.insert_or_assign(keyname, tbl);
  WriteTomlSettings(settings, flags);
  return TRUE;
}

// @426
BOOL STORMAPI SRegSaveValue(const char* keyname, const char* valuename, std::uint32_t flags, std::uint32_t value) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);
  tbl.insert_or_assign(valuename, value);

  settings.insert_or_assign(keyname, tbl);
  WriteTomlSettings(settings, flags);
  return TRUE;
}

// @427
BOOL STORMAPI SRegGetBaseKey(std::uint32_t flags, char* buffer, std::uint32_t buffersize) {
  if (buffer == nullptr || buffersize == 0) return FALSE;

  SStrCopy(buffer, SRegGetBaseKeyInternal(flags), buffersize);
  return TRUE;
}

// @428
BOOL STORMAPI SRegDeleteValue(const char* keyname, const char* valuename, std::uint32_t flags) {
  auto& settings = GetTomlSettings(flags);

  toml::table tbl = GetOrCreateTable(settings[keyname]);
  tbl.erase(valuename);

  settings.insert_or_assign(keyname, tbl);
  WriteTomlSettings(settings, flags);
  return TRUE;
}

// @429
BOOL STORMAPI SRegEnumKey(const char* keyname, std::uint32_t flags, std::uint32_t index, char* buffer, std::uint32_t bufferchars) {
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
BOOL STORMAPI SRegGetNumSubKeys(const char* keyname, std::uint32_t flags, std::uint32_t* subkeys) {
  auto& settings = GetTomlSettings(flags);
  
  toml::table tbl = GetOrCreateTable(settings[keyname]);
  if (subkeys) *subkeys = static_cast<std::uint32_t>(tbl.size());
  return TRUE;
}

