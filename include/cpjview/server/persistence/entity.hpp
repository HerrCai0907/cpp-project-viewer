#pragma once

#include "cpjview/server/persistence/string_pool.hpp"

namespace cpjview::persistence {

class IEntity {
public:
  virtual ~IEntity() = default;

  template <class T> T *cast() { return dynamic_cast<T *>(this); }
  template <class T> T const *cast() const {
    return dynamic_cast<T const *>(this);
  }
  template <class T> bool is() const { return cast<T>(this) != nullptr; }
};

class Node : public IEntity {};

class Relationship : public IEntity {
  Node *m_source;
  Node *m_target;

public:
  Relationship(Node *source, Node *target)
      : m_source(source), m_target(target) {}

  Node *get_source() { return m_source; }
  Node *get_target() { return m_target; }
};

class ClassNode;

class Inheritance;

class ClassNode : public Node {
  StringPool::StringIndex m_name;

public:
  explicit ClassNode(StringPool::StringIndex const &name) : m_name(name) {}
};

class Inheritance : public Relationship {
public:
  Inheritance(ClassNode *source, ClassNode *target)
      : Relationship(source, target) {}

  ClassNode *get_source() {
    return Relationship::get_source()->cast<ClassNode>();
  }
  ClassNode *get_target() {
    return Relationship::get_target()->cast<ClassNode>();
  }
};

} // namespace cpjview::persistence
