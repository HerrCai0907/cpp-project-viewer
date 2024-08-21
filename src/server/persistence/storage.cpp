#include "cpjview/server/persistence/storage.hpp"
#include "cpjview/server/persistence/entity.hpp"
#include "cpjview/server/persistence/error_code.hpp"
#include "cpjview/server/persistence/kv.hpp"
#include "cpjview/server/persistence/project.hpp"
#include "cpjview/server/persistence/string_pool.hpp"
#include "spdlog/spdlog.h"
#include <sqlite3.h>
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
      .ensure_relationship(derived_index, SymbolKind::ClassNode, base_index,
                           SymbolKind::ClassNode,
                           RelationshipKind::Inheritance);
}

Result<std::vector<Storage::InheritancePair>, ErrorCode>
Storage::get_inheritances(std::string const &project_name) const {
  StringPool::StringIndex project_name_index =
      m_impl->ensure_string_in_cache(project_name);
  using RT = Result<std::vector<Storage::InheritancePair>, ErrorCode>;

  Project *project = m_impl->m_project_map.get(project_name_index);
  if (project == nullptr) {
    return RT::failed(ErrorCode::not_found());
  }
  std::vector<Storage::InheritancePair> ret{};
  project->for_each_typed_relationship<Inheritance>(
      [&ret](const Inheritance *inheritance) -> void {
        ret.push_back({
            .derived =
                inheritance->get_source()->cast<ClassSymbol>()->get_name(),
            .base = inheritance->get_target()->cast<ClassSymbol>()->get_name(),
        });
      });
  return RT::success(std::move(ret));
}

// ================================================================
// =========================== Code ===============================
// ================================================================

void Storage::put_code(std::string const &project_name, std::string const &name,
                       std::string const &code) {
  StringPool::StringIndex project_name_index =
      m_impl->ensure_string_in_cache(project_name);
  StringPool::StringIndex name_index = m_impl->ensure_string_in_cache(name);
  StringPool::StringIndex code_index = m_impl->ensure_string_in_cache(code);

  m_impl->ensure_project(project_name_index)
      .ensure_relationship(name_index, SymbolKind::ClassNode, code_index,
                           SymbolKind::CodeNode, RelationshipKind::SourceCode);
}

Result<const char *, ErrorCode>
Storage::get_code(std::string const &project_name,
                  std::string const &name) noexcept {
  using RT = Result<const char *, ErrorCode>;

  StringPool::StringIndex project_name_index =
      m_impl->ensure_string_in_cache(project_name);
  StringPool::StringIndex name_index = m_impl->ensure_string_in_cache(name);

  Project *project = m_impl->m_project_map.get(project_name_index);
  if (project == nullptr) {
    return RT::failed(ErrorCode::not_found());
  }
  ClassSymbol *symbol = project->get_node(name_index)->dyn_cast<ClassSymbol>();
  if (symbol == nullptr) {
    spdlog::warn("[persistence] cannot find class symbol for {}", name);
    return RT::failed(ErrorCode::not_found());
  }

  Relationship *relationship =
      symbol->get_relationship(RelationshipKind::SourceCode);
  if (relationship == nullptr) {
    spdlog::warn("[persistence] cannot source code relationship for {}", name);
    return RT::failed(ErrorCode::not_found());
  }
  StringPool::StringIndex code = relationship->cast<SourceCode>()
                                     ->get_target()
                                     ->cast<CodeSymbol>()
                                     ->get_code();
  return RT::success(code);
}

// ================================================================
// ========================== Label ===============================
// ================================================================

Result<void, ErrorCode> Storage::put_label(std::string const &project_name,
                                           std::string const &symbol,
                                           protocol::LabelKind label) {
  using RT = Result<void, ErrorCode>;
  StringPool::StringIndex project_name_index =
      m_impl->ensure_string_in_cache(project_name);
  StringPool::StringIndex name_index = m_impl->ensure_string_in_cache(symbol);

  SymbolKind kind;
  switch (label) {
  case protocol::LabelKind::Record:
    kind = SymbolKind::ClassNode;
    break;
  default:
    return RT::failed(ErrorCode::not_found());
  }
  m_impl->ensure_project(project_name_index).ensure_node(name_index, kind);
  return RT::success();
}

// ================================================================
// ========================== Class ===============================
// ================================================================

Result<std::vector<const char *>, ErrorCode>
Storage::get_classes(std::string const &project_name) {
  using RT = Result<std::vector<const char *>, ErrorCode>;
  StringPool::StringIndex project_name_index =
      m_impl->ensure_string_in_cache(project_name);

  Project *project = m_impl->m_project_map.get(project_name_index);
  if (project == nullptr) {
    return RT::failed(ErrorCode::not_found());
  }
  std::vector<const char *> ret{};
  project->for_each_typed_node<ClassSymbol>(
      [&ret](const ClassSymbol *symbol) -> void {
        ret.push_back(symbol->get_name());
      });
  return RT::success(ret);
}

} // namespace cpjview::persistence
