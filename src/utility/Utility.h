#pragma once

#include <string>

namespace hydla{
namespace utility{

/**
 * get string of numerator and denominator
 * from given real value
 * !Warning! There is no insurance for accuracy
 * @return denominator is necessary
 * @param val given real value
 * @param numerator numerator string (output variable)
 * @param denominator denominator string (output variable)
 */
bool num_denom_str(double val,
                   std::string &numerator,
                   std::string &denominator);
/**
 * string version
 */
bool num_denom_str(std::string val_str,
                   std::string &numerator,
                   std::string &denominator);


/**
 * return string whose substrs are replaced to dest
 */
std::string replace(std::string original,
                    const std::string &substr,
                    const std::string &dest);

std::string to_string(int n);


/**
 * remove comment from given string
 * @return removed comment
 */
std::string remove_comment(std::string &src);


}  //  namespace utility
}  //  namespace hydla
