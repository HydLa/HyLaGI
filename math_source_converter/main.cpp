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

      // ƒRƒƒ“ƒgíœ
    case '(': {
      char nc = *in_itr++;
      if(nc == '*') {
	int count = 1;
	c  = *in_itr++;
	nc = *in_itr++;
	while(count > 0) {
	  if(c == '(' && nc == '*') {
	    count++;
	    c  = *in_itr++;
	    nc = *in_itr++;
	  } else if(c == '*' && nc == ')') {
	    count--;
	    c  = *in_itr++;
	    if(count>0) nc = *in_itr++;
	  } else {
	    c  = nc;
	    nc = *in_itr++;
	  }
	}
      } else {
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
