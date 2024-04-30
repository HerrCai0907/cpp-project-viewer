#include "cpjview/server/persistence/inheritance.hpp"
#include "cpjview/server/persistence/kv.hpp"
#include "cpjview/server/persistence/storage.hpp"
#include <set>

namespace cpjview::persistence {

namespace {} // namespace

void Inheritance::add_inheritance(const char *derived, const char *base) {
  m_search_map.modify(derived, [base](Info &info) -> ErrorCodeResult {
    info.m_base.insert(base);
    return ErrorCodeResult::success();
  });
  m_search_map.modify(base, [derived](Info &info) -> ErrorCodeResult {
    info.m_derived.insert(derived);
    return ErrorCodeResult::success();
  });
}

struct InheritancePair {
  const char *derived;
  const char *base;
};
std::vector<Storage::InheritancePair> Inheritance::get_all_inheritance() const {
  std::vector<Storage::InheritancePair> inheritancePairs{};
  for (auto [derived, relationship] : m_search_map) {
    for (const char *base : relationship.m_base) {
      inheritancePairs.push_back({.derived = derived, .base = base});
    }
  }
  return inheritancePairs;
}

} // namespace cpjview::persistence
