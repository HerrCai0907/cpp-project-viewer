#include "cpjview/server/persistence/project.hpp"
#include "cpjview/server/persistence/entity.hpp"
#include <cassert>
#include <functional>
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
  case SymbolKind::CodeNode:
    it->second.reset(new CodeSymbol(name));
    break;
  }
  assert(it->second != nullptr);
  return it->second.get();
}

void Project::ensure_relationship(Symbol *source, Symbol *target,
                                  RelationshipKind kind) {
  assert(source != target && "cannot create relationship with self");
  for (Relationship const *const r : source->get_relationships(kind)) {
    if (r->get_target() == target) {
      return;
    }
  }

  Relationship *relationship = nullptr;
  switch (kind) {
  case RelationshipKind::Inheritance: {
    auto inheritance = std::make_unique<Inheritance>(
        source->cast<ClassSymbol>(), target->cast<ClassSymbol>());
    relationship = inheritance.get();
    m_relationship_map.insert_or_assign(relationship, std::move(inheritance));
    break;
  }
  case RelationshipKind::SourceCode: {
    auto source_code = std::make_unique<SourceCode>(source->cast<ClassSymbol>(),
                                                    target->cast<CodeSymbol>());
    relationship = source_code.get();
    m_relationship_map.insert_or_assign(relationship, std::move(source_code));
    break;
  }
  };
  assert(relationship != nullptr);
  source->add_relationship(kind, relationship);
  target->add_relationship(kind, relationship);
}

void Project::ensure_relationship(StringPool::StringIndex source,
                                  SymbolKind source_kind,
                                  StringPool::StringIndex target,
                                  SymbolKind target_kind,
                                  RelationshipKind kind) {
  Symbol *source_node = ensure_node(source, source_kind);
  Symbol *target_node = ensure_node(target, target_kind);

  ensure_relationship(source_node, target_node, kind);
}

Symbol *Project::get_node(StringPool::StringIndex name) {
  std::unique_ptr<Symbol> *node = m_node_map.get(name);
  if (node != nullptr) {
    return node->get();
  }
  return nullptr;
}
void Project::for_each_node(std::function<void(Symbol const *)> fn) const {
  for (auto const &[_, symbol] : m_node_map) {
    fn(symbol.get());
  }
}

void Project::for_each_relationship(
    std::function<void(Relationship const *)> fn) const {
  for (auto const &[relationship, _] : m_relationship_map) {
    fn(relationship);
  }
}

} // namespace cpjview::persistence
