#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <iterator>
#include <algorithm>

int main()
{
	std::ifstream in_file("input.data");
	if(!in_file) {
		std::cout << "cannot open" << std::endl;
	}

	std::ofstream out_file("output.data");

	std::ostringstream out_str;

	std::istreambuf_iterator<char> in_itr(in_file);
	std::ostreambuf_iterator<char> out_itr(out_file);
	std::istreambuf_iterator<char> end_itr();
	
	//std::copy(in_itr, std::istreambuf_iterator<char>(), out_itr);

	while(in_itr != std::istreambuf_iterator<char>()) {
		char c = *in_itr++;
		switch(c) {
			case '\"':
				*out_itr++ = '\\';
				*out_itr++ = '\"';
				break;

			case '\\':
				*out_itr++ = '\\';
				*out_itr++ = '\\';
				break;

			case '\r':
				break;

			case '\n':
				*out_itr++ = '\\';
				*out_itr++ = 'n';
				break;

			default:
				*out_itr++ = c;
		}
	}

	//std::cout << out_str.str() << std::endl;

	in_file.close();
	out_file.close();

	return 0;
}