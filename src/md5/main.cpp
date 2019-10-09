#include "md5.hpp"
#include <iostream>

//md5(md5(<username>:<realm>:<password>):<nonce>:md5(<cmd>:<url>))

/*
test:
md5(md5("admin":"LIVE555 Streaming Media" : "admin"):"6d8ddf69907708a0c0a71a451759644f" : md5("DESCRIBE":"rtsp://192.168.52.131:554/test.264"))
77693fac318b4716dac49f16612bdab5
*/

int main(int argc, char **argv)
{
	auto hex1 = md5::md5_hash_hex("admin:LIVE555 Streaming Media:admin");
	auto hex2 = md5::md5_hash_hex("DESCRIBE:rtsp://192.168.52.131:554/test.264");
	auto hex3 = md5::md5_hash_hex(hex1 + ":6d8ddf69907708a0c0a71a451759644f:" + hex2);
	if (hex3 == "77693fac318b4716dac49f16612bdab5")
	{
		std::cout << hex3 << std::endl;
	}
	
	std::cout << "test generate nonce: " << md5::generate_nonce() << std::endl;
	std::cout << "test generate nonce: " << md5::generate_nonce() << std::endl;
	std::cout << "test generate nonce: " << md5::generate_nonce() << std::endl;

	getchar();
	return 0;
}