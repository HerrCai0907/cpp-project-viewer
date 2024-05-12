#include "cpjview/server/persistence/project.hpp"
#include "cpjview/server/persistence/entity.hpp"
#include <cassert>
#include <memory>

namespace cpjview::persistence {

Symbol *Project::ensure_node(StringPool::StringIndex name, SymbolKind kind) {
  std::unique_ptr<Symbol> *node = m_node_map.get(name);
  if (node != nullptr) {
    return node->get();
  }
  auto it = m_node_map.renew(name);
  switch (kind) {
  case SymbolKind::ClassNode:
    it->second.reset(new ClassSymbol(name));
    break;
  }
  return it->second.get();
}

void Project::ensure_relationship(Symbol *source, Symbol *target,
                                  RelationshipKind kind) {
  Relationship *relationship = nullptr;
  switch (kind) {
  case RelationshipKind::Inheritance:
    auto inheritance = std::make_unique<Inheritance>(
        source->cast<ClassSymbol>(), target->cast<ClassSymbol>());
    relationship = inheritance.get();
    m_relationship_map.insert_or_assign(relationship, std::move(inheritance));
    break;
  };
  assert(relationship != nullptr);
  source->add_relationship(kind, relationship);
  target->add_relationship(kind, relationship);
}

void Project::ensure_relationship(StringPool::StringIndex source,
                                  StringPool::StringIndex target,
                                  RelationshipKind kind) {
  auto source_node = ensure_node(source, SymbolKind::ClassNode);
  auto target_node = ensure_node(target, SymbolKind::ClassNode);
  ensure_relationship(source_node, target_node, kind);
}

} // namespace cpjview::persistence
