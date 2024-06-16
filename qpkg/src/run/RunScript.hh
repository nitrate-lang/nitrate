#ifndef __QPKG_RUN_RUNSCRIPT_HH__
#define __QPKG_RUN_RUNSCRIPT_HH__

#include <build/Engine.hh>
#include <set>
#include <string>

namespace qpkg {
namespace run {
//
class RunScript {
  std::string m_scriptfile;

 public:
  RunScript(const std::string &scriptfile);

  bool is_okay() const;

  int run(const std::vector<std::string> &args);
};

}  // namespace run
}  // namespace qpkg

#endif  // __QPKG_RUN_RUNSCRIPT_HH__
