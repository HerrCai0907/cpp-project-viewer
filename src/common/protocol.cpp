#include "cpjview/protocol/code.hpp"
#include "cpjview/protocol/inheritance.hpp"
#include "cpjview/protocol/label.hpp"
#include "cpjview/protocol/project.hpp"
#include <cassert>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace cpjview::protocol {

std::string Inheritance::to_json() const {
  return nlohmann::json{
      {"derived", m_derived},
      {"base", m_base},
  }
      .dump();
}

Inheritance Inheritance::from_json(std::string json_str) {
  const nlohmann::json json = nlohmann::json::parse(json_str);
  return Inheritance{
      .m_derived = json["derived"].get<std::string>(),
      .m_base = json["base"].get<std::string>(),
  };
}

std::string Project::to_json() const {
  return nlohmann::json{
      {"name", m_name},
  }
      .dump();
}

Project Project::from_json(std::string json_str) {
  const nlohmann::json json = nlohmann::json::parse(json_str);
  return Project{.m_name = json["name"].get<std::string>()};
}

std::string SourceCode::to_json() const {
  return nlohmann::json{
      {m_name, m_code},
  }
      .dump();
}

SourceCode SourceCode::from_json(std::string json_str) {
  const nlohmann::json json = nlohmann::json::parse(json_str);
  assert(json.size() == 1);
  return SourceCode{
      .m_name = json.items().begin().key(),
      .m_code = json.items().begin().value().get<std::string>(),
  };
}

std::string Label::to_json() const {
  return nlohmann::json{
      {m_symbol, static_cast<uint32_t>(m_label)},
  }
      .dump();
}

Label Label::from_json(std::string json_str) {
  const nlohmann::json json = nlohmann::json::parse(json_str);
  assert(json.size() == 1);
  return Label{
      .m_symbol = json.items().begin().key(),
      .m_label =
          static_cast<LabelKind>(json.items().begin().value().get<uint32_t>()),
  };
}

} // namespace cpjview::protocol
