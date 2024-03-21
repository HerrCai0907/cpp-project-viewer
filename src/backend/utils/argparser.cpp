#include "cpjview/utils/argparser.hpp"
#include <cassert>

namespace cpjview {

void ArgParser::add_option(std::string &option_value, const char *name,
                           const char *desc) {
  bool success = m_options
                     .insert_or_assign(name,
                                       [this, &option_value](
                                           llvm::ArrayRef<const char *> &argv) {
                                         option_value = argv[0];
                                         argv = argv.drop_front();
                                       })
                     .second;

  assert(success);
}

void ArgParser::add_option(std::vector<std::string> &option_value,
                           const char *name, const char *desc) {
  bool success = m_options
                     .insert_or_assign(name,
                                       [this, &option_value](
                                           llvm::ArrayRef<const char *> &argv) {
                                         option_value.push_back(argv[0]);
                                         argv = argv.drop_front();
                                       })
                     .second;

  assert(success);
}

Result<void, std::string> ArgParser::parse(llvm::ArrayRef<const char *> argv) {
  argv = argv.drop_front(); // ignore current process name
  while (!argv.empty()) {
    auto it = m_options.find(argv[0]);
    if (it != m_options.end()) {
      argv = argv.drop_front();
      it->second(argv);
      continue;
    }
    return Result<void, std::string>::failed(
        std::string{"unknown command option: "} + argv[0]);
  }
  return Result<void, std::string>::success();
}

} // namespace cpjview