#pragma once

#include <map>
#include <utility>
namespace cpjview::persistence {

template <class E> class SearchMap {
  using Map = std::map<const char *, E>;
  Map m_map{};

public:
  auto renew(const char *key) {
    return m_map.insert_or_assign(key, E{}).second;
  }

  void modify(const char *key, std::function<void(E &)> &&fn) {
    typename Map::iterator it = m_map.find(key);
    if (it == m_map.end()) {
      it = m_map.emplace(key, E{});
    }
    std::forward(fn)(it.second);
  }
};

} // namespace cpjview::persistence