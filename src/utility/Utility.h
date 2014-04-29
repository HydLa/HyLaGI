#ifndef _INCLUDED_HYDLA_UTILITY_H_
#define _INCLUDED_HYDLA_UTILITY_H_

#include <string>

namespace hydla{
namespace utility{

/**
 * get string of numerator and denominator
 * from given real value
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

std::string to_string(int n);

}  //  namespace utility
}  //  namespace hydla

#endif  // _INCLUDED_HYDLA_UTILITY_H_
