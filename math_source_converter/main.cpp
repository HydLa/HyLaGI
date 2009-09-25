#include <iostream>
#include <strstream>
#include <iterator>
#include <algorithm>

void conv()
{
  std::istreambuf_iterator<char> in_itr(std::cin);
  std::ostreambuf_iterator<char> out_itr(std::cout);
  
  *out_itr++ = '\"';
  
  while(in_itr != std::istreambuf_iterator<char>()) {
    char c = *in_itr++;
    switch(c) {
    case '\"':
      *out_itr++ = '\\';
      *out_itr++ = '\\';
      *out_itr++ = '\\';
      *out_itr++ = '\"';
      break;
      
    case '\\':
      *out_itr++ = '\\';
      *out_itr++ = '\\';
      *out_itr++ = '\\';
      *out_itr++ = '\\';
      break;
      
    case '\r':
      break;
      
    case '\n':
      *out_itr++ = '\\';
      *out_itr++ = '\\';
      *out_itr++ = '\\';
      *out_itr++ = 'n';
      *out_itr++ = '\"';
      *out_itr++ = '\n';
      *out_itr++ = '\"';
      break;

    default:
      *out_itr++ = c;
    }
  }

  *out_itr++ = '\"';
}

int main()
{
  std::cout << 
    "#include \"math_source.h\"\n\n"
    "const char* math_source() {\n"
    "  return \n";
  conv();
  std::cout << 
    ";\n"
    "}";

  return 0;
}
