#include "cpjview/protocol/inheritance.hpp"
#include <nlohmann/json.hpp>

namespace cpjview::protocol {

std::string Inheritance::to_json() const {
  return nlohmann::json{
      {"derived", m_derived},
      {"base", m_base},
  }
      .dump();
}

Inheritance Inheritance::from_json(std::string json_str) {
  nlohmann::json json = nlohmann::json::parse(json_str);
  return Inheritance{
      .m_derived = json["derived"].get<std::string>(),
      .m_base = json["base"].get<std::string>(),
  };
}

} // namespace cpjview::protocol
