#include "cpjview/server/persistence/storage.hpp"
#include "cpjview/server/persistence/inheritance.hpp"
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
  Inheritance m_inheritance{};

public:
  void add_inheritance(std::string const &derived, std::string const &base) {
    const char *base_c_str = ensure_string_in_cache(base);
    const char *derived_c_str = ensure_string_in_cache(derived);
    m_inheritance.add_inheritance(derived_c_str, base_c_str);
  }

  std::vector<Storage::InheritancePair> get_all_inheritance() const {
    return m_inheritance.get_all_inheritance();
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

void Storage::add_inheritance(std::string const &derived,
                              std::string const &base) {
  m_impl->add_inheritance(derived, base);
}

std::vector<Storage::InheritancePair> Storage::get_all_inheritance() const {
  return m_impl->get_all_inheritance();
}

} // namespace cpjview::persistence
