#include "cpjview/server/persistence/inheritance.hpp"
#include "cpjview/server/persistence/storage.hpp"
#include <map>
#include <set>

namespace cpjview::persistence {

namespace {} // namespace

void Inheritance::add_inheritance(const char *derived, const char *base) {
  auto getRelationship = [this](const char *str) -> Index & {
    Map::iterator derived_it = m_inheritance.find(str);
    if (derived_it != m_inheritance.end()) {
      return derived_it->second;
    }
    return m_inheritance.insert_or_assign(str, Index{}).first->second;
  };
  getRelationship(base).m_derived.insert(derived);
  getRelationship(derived).m_base.insert(base);
}

struct InheritancePair {
  const char *derived;
  const char *base;
};
std::vector<Storage::InheritancePair> Inheritance::get_all_inheritance() const {
  std::vector<Storage::InheritancePair> inheritancePairs{};
  for (auto [derived, relationship] : m_inheritance) {
    for (const char *base : relationship.m_base) {
      inheritancePairs.push_back({.derived = derived, .base = base});
    }
  }
  return inheritancePairs;
}

} // namespace cpjview::persistence
