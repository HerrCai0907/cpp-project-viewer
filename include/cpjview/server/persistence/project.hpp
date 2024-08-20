#pragma once

#include "cpjview/server/persistence/entity.hpp"
#include "cpjview/server/persistence/kv.hpp"
#include "cpjview/server/persistence/string_pool.hpp"
#include <functional>
#include <map>
#include <memory>
#include <type_traits>

namespace cpjview::persistence {

class Project {
  SearchMap<std::unique_ptr<Symbol>> m_node_map{};
  std::map<Relationship *, std::unique_ptr<Relationship>> m_relationship_map{};

public:
  Symbol *ensure_node(StringPool::StringIndex name, SymbolKind kind);

  void ensure_relationship(Symbol *source, Symbol *target,
                           RelationshipKind kind);
  void ensure_relationship(StringPool::StringIndex source,
                           SymbolKind source_kind,
                           StringPool::StringIndex target,
                           SymbolKind target_kind, RelationshipKind kind);

  Symbol *get_node(StringPool::StringIndex name);
  void for_each_node(std::function<void(Symbol const *)> fn) const;
  template <class T>
    requires(std::is_base_of_v<Symbol, T>)
  void for_each_typed_node(std::function<void(T const *)> fn) const {
    for_each_node([&](Symbol const *symbol) {
      const T *typed = symbol->dyn_cast<T>();
      if (typed == nullptr) {
        return;
      }
      fn(typed);
    });
  }

  void
  for_each_relationship(std::function<void(Relationship const *)> fn) const;

  template <class T>
    requires(std::is_base_of_v<Relationship, T>)
  void for_each_typed_relationship(std::function<void(T const *)> fn) const {
    for_each_relationship([&](Relationship const *relationship) {
      const T *typed = relationship->dyn_cast<T>();
      if (typed == nullptr) {
        return;
      }
      fn(typed);
    });
  }
};

} // namespace cpjview::persistence
