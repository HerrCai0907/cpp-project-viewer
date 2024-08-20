#pragma once

#include "cpjview/server/persistence/entity.hpp"
#include "cpjview/server/persistence/kv.hpp"
#include "cpjview/server/persistence/string_pool.hpp"
#include <functional>
#include <map>
#include <memory>

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

  void
  for_each_relationship(std::function<void(Relationship const *)> fn) const;
};

} // namespace cpjview::persistence
