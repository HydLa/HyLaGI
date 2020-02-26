#include <iostream>
#include <iterator>

void conv() {
  std::istreambuf_iterator<char> in_itr(std::cin);
  std::ostreambuf_iterator<char> out_itr(std::cout);

  *out_itr++ = '\"';

  char c = *in_itr++;
  while (in_itr != std::istreambuf_iterator<char>()) {
    switch (c) {
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

    // for comment
    case '(': {
      char nc = *in_itr++;
      if (nc == '*') {
        int count = 1;
        c = *in_itr++;
        nc = *in_itr++;
        while (count > 0) {
          if (c == '(' && nc == '*') {
            count++;
            c = *in_itr++;
            nc = *in_itr++;
          } else if (c == '*' && nc == ')') {
            count--;
            c = *in_itr++;
            if (count > 0)
              nc = *in_itr++;
          } else {
            c = nc;
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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "arg size must be 2" << std::endl;
    return -1;
  }

  std::cout << "#include \"" << argv[1]
            << ".h\"\n\n"
               "namespace hydla{\n"
               "namespace backend{\n"
               "namespace mathematica{\n"
               "const char* "
            << argv[1]
            << "() {\n"
               "  return \n";
  conv();
  std::cout << ";\n"
               "}\n"
               "} //mathematica\n"
               "} //backend\n"
               "} //hydla\n";

  return 0;
}
