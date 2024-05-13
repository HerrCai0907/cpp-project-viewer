#include "cpjview/server/persistence/storage.hpp"
#include "cpjview/server/persistence/entity.hpp"
#include "cpjview/server/persistence/error_code.hpp"
#include "cpjview/server/persistence/kv.hpp"
#include "cpjview/server/persistence/project.hpp"
#include "cpjview/server/persistence/string_pool.hpp"
#include <string>

namespace cpjview::persistence {

struct StorageImpl {
  StringPool m_string_cache{};
  SearchMap<Project> m_project_map;

  Project &ensure_project(StringPool::StringIndex name) {
    return m_project_map.try_add(name)->second;
  }
  StringPool::StringIndex ensure_string_in_cache(std::string_view str) {
    return m_string_cache.ensure_string_in_cache(str);
  }
};

Storage::Storage() { m_impl = new StorageImpl(); }

Storage::~Storage() { delete m_impl; }

// ================================================================
// ========================= Project ==============================
// ================================================================

ErrorCodeResult Storage::put_project(std::string const &name) {
  StringPool::StringIndex name_index = m_impl->ensure_string_in_cache(name);
  m_impl->m_project_map.renew(name_index);
  return ErrorCodeResult::success();
}

std::vector<const char *> Storage::get_projects() const {
  std::vector<const char *> list{};
  for (auto &[name, _] : m_impl->m_project_map) {
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
  StringPool::StringIndex project_name_index =
      m_impl->ensure_string_in_cache(project_name);
  StringPool::StringIndex base_index = m_impl->ensure_string_in_cache(base);
  StringPool::StringIndex derived_index =
      m_impl->ensure_string_in_cache(derived);

  m_impl->ensure_project(project_name_index)
      .ensure_relationship(derived_index, base_index,
                           RelationshipKind::Inheritance);
}

Result<std::vector<Storage::InheritancePair>, ErrorCode>
Storage::get_all_inheritance(std::string const &project_name) const {
  StringPool::StringIndex project_name_index =
      m_impl->ensure_string_in_cache(project_name);
  using RT = Result<std::vector<Storage::InheritancePair>, ErrorCode>;

  Project *project = m_impl->m_project_map.get(project_name_index);
  if (project == nullptr) {
    return RT::failed(ErrorCode::not_found());
  }
  std::vector<Storage::InheritancePair> ret{};
  project->for_each_relationship([&ret](
                                     const Relationship *relationship) -> void {
    const Inheritance *inheritance = relationship->dyn_cast<Inheritance>();
    if (inheritance == nullptr) {
      return;
    }
    ret.push_back({
        .derived = inheritance->get_source()->cast<ClassSymbol>()->get_name(),
        .base = inheritance->get_target()->cast<ClassSymbol>()->get_name(),
    });
  });
  return RT::success(std::move(ret));
}

} // namespace cpjview::persistence
