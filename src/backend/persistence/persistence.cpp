#include "cpjview/persistence/persistence.hpp"
#include <map>
#include <set>
#include <string>

namespace cpjview::persistence {

namespace {
struct InheritanceRelationship {
  std::set<const char *> m_derived{};
  std::set<const char *> m_base{};
};
} // namespace

class StorageImpl {
  using StringCacheType = std::set<const std::string>;
  StringCacheType m_string_cache{};
  using InheritanceRelationshipMap =
      std::map<const char *, InheritanceRelationship>;
  InheritanceRelationshipMap m_inheritance{};

public:
  void add_inheritance_relationship(std::string_view derived,
                                    std::string_view base) {
    const char *base_c_str = ensure_string_in_cache(base);
    const char *derived_c_str = ensure_string_in_cache(derived);
    auto getRelationship =
        [this](const char *str) -> InheritanceRelationship & {
      InheritanceRelationshipMap::iterator derived_it = m_inheritance.find(str);
      if (derived_it != m_inheritance.end()) {
        return derived_it->second;
      }
      return m_inheritance.insert_or_assign(str, InheritanceRelationship{})
          .first->second;
    };
    getRelationship(base_c_str).m_derived.insert(derived_c_str);
    getRelationship(derived_c_str).m_base.insert(base_c_str);
  }

  std::vector<Storage::InheritancePair> get_inheritance_relationships() const {
    std::vector<Storage::InheritancePair> inheritancePairs{};
    for (auto [derived, relationship] : m_inheritance) {
      for (const char *base : relationship.m_base) {
        inheritancePairs.push_back({.derived = derived, .base = base});
      }
    }
    return inheritancePairs;
  }

private:
  const char *ensure_string_in_cache(std::string_view str) {
    StringCacheType::const_iterator it =
        m_string_cache.insert(std::string{str}).first;
    return it->c_str();
  }
};

Storage::Storage() { m_impl = new StorageImpl(); }

Storage::~Storage() { delete m_impl; }

void Storage::add_inheritance_relationship(std::string_view derived,
                                           std::string_view base) {
  m_impl->add_inheritance_relationship(derived, base);
}

std::vector<Storage::InheritancePair>
Storage::get_inheritance_relationships() const {
  return m_impl->get_inheritance_relationships();
}

} // namespace cpjview::persistence
