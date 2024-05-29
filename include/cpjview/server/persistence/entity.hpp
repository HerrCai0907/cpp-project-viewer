#pragma once

#include "cpjview/server/persistence/string_pool.hpp"
#include <cassert>
#include <cstddef>
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
  template <class T> bool is() const { return dyn_cast<T>() != nullptr; }
};

enum class SymbolKind {
  ClassNode,
  CodeNode,
};

enum class RelationshipKind {
  Inheritance, // derived -> base
  SourceCode,  // name -> code
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

  class IterableRelationships {
    std::list<Relationship *> const *m_list;

  public:
    explicit IterableRelationships(std::list<Relationship *> const *list)
        : m_list(list) {}

    std::list<Relationship *>::const_iterator begin() {
      if (m_list == nullptr) {
        return {};
      }
      return m_list->begin();
    }
    std::list<Relationship *>::const_iterator end() {
      if (m_list == nullptr) {
        return {};
      }
      return m_list->end();
    }
  };

  IterableRelationships get_relationships(RelationshipKind kind) const {
    auto it = m_relationships.find(kind);
    if (it == m_relationships.end()) {
      return IterableRelationships{nullptr};
    }
    return IterableRelationships{&it->second};
  }

  Relationship *get_relationship(RelationshipKind kind) const {
    auto it = m_relationships.find(kind);
    if (it == m_relationships.end()) {
      return nullptr;
    }
    size_t size = it->second.size();
    if (size == 0) {
      return nullptr;
    }
    assert(size == 1);
    return it->second.front();
  }
};

class Relationship : public IEntity {
  Symbol *m_source;
  Symbol *m_target;

public:
  Relationship(Symbol *source, Symbol *target)
      : m_source(source), m_target(target) {}

  Symbol *get_source() const { return m_source; }
  Symbol *get_target() const { return m_target; }
};

class ClassSymbol : public Symbol {
  StringPool::StringIndex m_name;

public:
  explicit ClassSymbol(StringPool::StringIndex const &name) : m_name(name) {}

  StringPool::StringIndex get_name() const { return m_name; }
};

class CodeSymbol : public Symbol {
  StringPool::StringIndex m_code;

public:
  explicit CodeSymbol(StringPool::StringIndex const &code) : m_code(code) {}

  StringPool::StringIndex get_code() const { return m_code; }
};

class Inheritance : public Relationship {
public:
  Inheritance(ClassSymbol *source, ClassSymbol *target)
      : Relationship(source, target) {}

  ClassSymbol *get_source() const {
    return Relationship::get_source()->cast<ClassSymbol>();
  }
  ClassSymbol *get_target() const {
    return Relationship::get_target()->cast<ClassSymbol>();
  }
};

class SourceCode : public Relationship {
public:
  SourceCode(ClassSymbol *name, CodeSymbol *code) : Relationship(name, code) {}
};

} // namespace cpjview::persistence
