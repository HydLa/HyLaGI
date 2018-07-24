#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

#ifdef _MSC_VER
#include <filesystem>
#endif

void conv()
{
  std::istreambuf_iterator<char> in_itr(std::cin);
  std::ostreambuf_iterator<char> out_itr(std::cout);

#ifdef _MSC_VER
  *out_itr++ = '<';
  *out_itr++ = '<';
#endif

  *out_itr++ = '\"';

  char c = *in_itr++;
  while (in_itr != std::istreambuf_iterator<char>())
  {
    switch(c)
    {
      case '\"':
        *out_itr++ = '\\';
        *out_itr++ = '\"';
        c = *in_itr++;
        break;
      
      case '\\':
        *out_itr++ = '\\';
        *out_itr++ = '\\';
        c = *in_itr++;
        break;
      
      case '\r':
        c = *in_itr++;
        break;
      
      case '\n':
        *out_itr++ = '\\';
        *out_itr++ = 'n';
        *out_itr++ = '\"';
        *out_itr++ = '\n';
#ifdef _MSC_VER
        *out_itr++ = '<';
        *out_itr++ = '<';
#endif
        *out_itr++ = '\"';
        c = *in_itr++;   
        break;

      // for comment
      case '(': {
        char nc = *in_itr++;
        if (nc == '*')
        {
          int count = 1;
          c  = *in_itr++;
          nc = *in_itr++;
          while (count > 0)
          {
            if (c == '(' && nc == '*')
            {
              count++;
              c  = *in_itr++;
              nc = *in_itr++;
            }
            else if (c == '*' && nc == ')')
            {
              count--;
              c  = *in_itr++;
              if (count > 0) nc = *in_itr++;
            }
            else
            {
              c  = nc;
              nc = *in_itr++;
            }
          }
        }
        else
        {
          *out_itr++ = c;
          c = nc;
        }
        break;
      }

      default:
        *out_itr++ = c;
        c = *in_itr++;
    }
  }

  *out_itr++ = '\"';

#ifdef _MSC_VER
  *out_itr++ = ';';
#endif
}

inline std::vector<std::string> SplitStringVSCompatible(const std::string& str)
{
  std::vector<std::string> result;

  //const int divLength = 16380;
  const int divLength = 2000;
  while (result.size() * divLength < str.length())
  {
    const size_t offset = result.size() * divLength;
    result.push_back(std::string(str.begin() + offset, str.begin() + std::min(offset + divLength, str.length())));
  }

  return result;
}

int main(int argc, char *argv[])
{
  /*if (argc != 2)
  {
    std::cerr << "arg size must be 2" << std::endl;
    return -1;
  }*/

#ifdef _MSC_VER
  namespace fs = std::experimental::filesystem;
  std::stringstream sourceStream;

  for (int i = 1; i < argc; ++i)
  {
  for(const auto& ent : fs::recursive_directory_iterator(argv[i]))
  {
    //std::cout << ent.path().extension();
    if (fs::is_regular_file(ent) && ent.path().extension() == ".m")
    {
      std::ifstream ifs(fs::canonical(ent.path()).string());
      sourceStream << std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
    }
  }
  }

  /*
  for (const auto& ent : fs::recursive_directory_iterator("../backend/mathematica/math_source"))
  {
    std::cout << ent.path().extension();
    if (fs::is_regular_file(ent) && ent.path().extension() == ".m")
    {
      std::ifstream ifs(fs::canonical(ent.path()).string());
      sourceStream << std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
    }
  }
  */
  std::ofstream ofs("result.cpp");

  std::string sourceStr;
  for (char c : sourceStream.str())
  {
    if (c == '\n')
    {
      sourceStr += "\\n";
    }
    sourceStr += c;
  }

  const auto resultStrs = SplitStringVSCompatible(sourceStr);

  ofs <<
    "#include <sstream>\n"
    "#include \"math_source.h\"\n\n"
    "namespace hydla{\n"
    "namespace backend{\n"
    "namespace mathematica{\n"
    "const char* math_source() {\n"
    "  std::stringstream ss;\n"
    "  ss \n";
  //conv();
  for (const auto& str : resultStrs)
  {
	  ofs << "<< std::string(R\"(" << str << ")\")";
  }
  ofs <<
    ";\n"
    "  return ss.str().c_str(); \n"
    "}\n"
    "} //mathematica\n"
    "} //backend\n"
    "} //hydla\n";
#else
  std::cout <<
    "#include \"math_source.h\"\n\n"
    "namespace hydla{\n"
    "namespace backend{\n"
    "namespace mathematica{\n"
    "const char* math_source() {\n"
    "  return \n";
  conv();
  std::cout << 
    ";\n"
    "}\n"
    "} //mathematica\n"
    "} //backend\n"
    "} //hydla\n";
#endif

  return 0;
}
