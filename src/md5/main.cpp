#include "md5.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	std::cout << md5::md5_hash_hex("abcdefg") << std::endl;

	getchar();
	return 0;
}