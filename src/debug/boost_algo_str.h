#include <string>
#include <vector>

namespace hydla {
namespace debug {
/// Boostで実装されているが簡単なので実装\n
/// 将来的に標準入りしたら消すこと
void replace_all(std::string &str, const std::string &src,
                 const std::string &dst);
/// Boostで実装されているが簡単なので実装\n
/// 将来的に標準入りしたら消すこと
void split(std::vector<std::string> &dst, const std::string &src,
           const std::string &delim);
} // namespace debug
} // namespace hydla
