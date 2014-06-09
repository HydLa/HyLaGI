#include "Utility.h"
#include <sstream>
#include <iomanip>

using namespace std;

namespace hydla
{
namespace utility
{

bool num_denom_str(double val,
                   string &numerator,
                   string &denominator)
{
  stringstream sstr;
  // TODO: carefully handle precision
  sstr << setiosflags(ios::fixed) << val;
  return num_denom_str(sstr.str(), numerator, denominator);
}

bool num_denom_str(string val_str,
                   string &numerator,
                   string &denominator)
{
  string::size_type sz = val_str.find(".");
  denominator = "1";
  if(sz == string::npos)
  {
    numerator = val_str;
    return false;
  }
  else
  {
    denominator.append(val_str.size() - (sz + 1), '0');

    numerator = val_str.substr(0, sz) + val_str.substr(sz+1);

    // trim preceeding zeros
    string::size_type zero_cnt = 
      numerator.find_first_not_of('0');
    if(zero_cnt != string::npos)
    {
      numerator = numerator.substr(zero_cnt);
    }
    return true;
  }
}

string to_string(int n)
{
  stringstream sstr;
  sstr << n;
  return sstr.str();
}


string replace(string original, const string &substr, const string &dest )
{
  string::size_type  pos( original.find( substr ) );

  while( pos != string::npos )
  {
    original.replace( pos, substr.length(), dest );
    pos = original.find( substr, pos + dest.length() );
  }

  return original;
}


}
}
