#include <string>
#include <vector>

namespace hydla {
namespace debug {
void replace_all(std::string &str, const std::string &src,
                 const std::string &dst);
void split(std::vector<std::string> &dst, const std::string &src,
           const std::string &delim);
} // namespace debug
} // namespace hydla
