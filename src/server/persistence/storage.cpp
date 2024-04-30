#include "cpjview/server/persistence/storage.hpp"
#include "cpjview/server/persistence/error_code.hpp"
#include "cpjview/server/persistence/kv.hpp"
#include "cpjview/server/persistence/project.hpp"
#include <set>
#include <string>

namespace cpjview::persistence {

struct StorageImpl {
  using StringCacheType = std::set<const std::string>;
  StringCacheType m_string_cache{};
  Project m_projects{};

  const char *ensure_string_in_cache(std::string_view str) {
    StringCacheType::const_iterator it =
        m_string_cache.insert(std::string{str}).first;
    return it->c_str();
  }
};

Storage::Storage() { m_impl = new StorageImpl(); }

Storage::~Storage() { delete m_impl; }

// ================================================================
// ========================= Project ==============================
// ================================================================

ErrorCodeResult Storage::put_project(std::string const &name) {
  const char *str = m_impl->ensure_string_in_cache(name);
  return m_impl->m_projects.force_create(str);
}

std::vector<const char *> Storage::get_projects() const {
  std::vector<const char *> list{};
  for (auto [name, _] : m_impl->m_projects) {
    list.push_back(name);
  }
  return list;
}

// ================================================================
// ======================= Inheritance ============================
// ================================================================

void Storage::add_inheritance(std::string const &project_name,
                              std::string const &derived,
                              std::string const &base) {
  const char *base_c_str = m_impl->ensure_string_in_cache(base);
  const char *derived_c_str = m_impl->ensure_string_in_cache(derived);
  m_impl->m_projects.modify_info(
      project_name.c_str(),
      [base_c_str, derived_c_str](Project::Info &info) -> ErrorCodeResult {
        info.m_inheritance.add_inheritance(derived_c_str, base_c_str);
        return ErrorCodeResult::success();
      });
}

Result<std::vector<Storage::InheritancePair>, ErrorCode>
Storage::get_all_inheritance(std::string const &project_name) const {
  using RetType = Result<std::vector<Storage::InheritancePair>, ErrorCode>;
  Project::Info *info = m_impl->m_projects.get_info(project_name.c_str());
  if (info == nullptr) {
    return RetType::failed(ErrorCode::not_found());
  }
  return RetType::success(info->m_inheritance.get_all_inheritance());
}

} // namespace cpjview::persistence
