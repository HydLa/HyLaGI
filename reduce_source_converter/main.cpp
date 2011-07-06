#include <iostream>
#include <strstream>
#include <iterator>
#include <algorithm>

void conv()
{
  std::istreambuf_iterator<char> in_itr(std::cin);
  std::ostreambuf_iterator<char> out_itr(std::cout);
  
  *out_itr++ = '\"';
  
  char c = *in_itr++;
  while(in_itr != std::istreambuf_iterator<char>()) {
    switch(c) {
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
        *out_itr++ = '\"';
        c = *in_itr++;   
        break;

        // �R�����g�폜
      case '%': {
        while(c != '\n') c = *in_itr++;
        break;    
      }

      default:
        *out_itr++ = c;
        c = *in_itr++;
    }
  }

  *out_itr++ = '\"';
}

int main(int argc, char *argv[])
{
  if(argc != 2) {
    std::cerr << "arg size must be 2" << std::endl;
    return -1;
  }

  std::cout <<
    "#include \"" << argv[1] << ".h\"\n\n"
    "const char* " << argv[1] << "() {\n"
    "  return \n";
  conv();
  std::cout << 
    ";\n"
    "}";

  return 0;
}
