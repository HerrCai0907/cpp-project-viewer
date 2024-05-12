#pragma once

#include "cpjview/server/persistence/string_pool.hpp"
#include <cassert>
#include <list>
#include <map>

namespace cpjview::persistence {

class IEntity {
public:
  virtual ~IEntity() = default;

  template <class T> T *cast() {
    assert(is<T>());
    return dyn_cast<T>();
  }
  template <class T> T const *cast() const {
    assert(is<T>());
    return dyn_cast<T>();
  }
  template <class T> T *dyn_cast() { return dynamic_cast<T *>(this); }
  template <class T> T const *dyn_cast() const {
    return dynamic_cast<T const *>(this);
  }
  template <class T> bool is() const { return dyn_cast<T>(this) != nullptr; }
};

enum class SymbolKind {
  ClassNode,
};

enum class RelationshipKind {
  Inheritance,
};

class Symbol;
class Relationship;

class Symbol : public IEntity {
  std::map<RelationshipKind, std::list<Relationship *>> m_relationships{};

public:
  void add_relationship(RelationshipKind kind, Relationship *relationship) {
    m_relationships.try_emplace(kind, std::list<Relationship *>{})
        .first->second.push_back(relationship);
  }
};

class Relationship : public IEntity {
  Symbol *m_source;
  Symbol *m_target;

public:
  Relationship(Symbol *source, Symbol *target)
      : m_source(source), m_target(target) {}

  Symbol *get_source() { return m_source; }
  Symbol *get_target() { return m_target; }
};

class ClassSymbol;

class Inheritance;

class ClassSymbol : public Symbol {
  StringPool::StringIndex m_name;

public:
  explicit ClassSymbol(StringPool::StringIndex const &name) : m_name(name) {}
};

class Inheritance : public Relationship {
public:
  Inheritance(ClassSymbol *source, ClassSymbol *target)
      : Relationship(source, target) {}

  ClassSymbol *get_source() {
    return Relationship::get_source()->cast<ClassSymbol>();
  }
  ClassSymbol *get_target() {
    return Relationship::get_target()->cast<ClassSymbol>();
  }
};

} // namespace cpjview::persistence
