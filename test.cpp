#include <iostream>
#include "text_garbling_fixer.hpp"

int main(){
	std::string sample_text = 
		"Hello, world!: this -> \\u+52D5\\u+4F5C\\u+78BA\\u+8A8D\\u+7528\\u+30C6\\u+30AD\\u+30B9\\u+30C8 <- is a sample of converted text";
	std::cout
		<< encoding_fixer::convertUnicode(sample_text) << std::endl;

	std::cout << std::endl;

	std::string sample_text_sjis = 
		"\x82\xb1\x82\xea\x82\xcd\x53\x68\x69\x66\x74\x2d\x4a\x49\x53\x82\xa9\x82\xe7\x95\xcf\x8a\xb7\x82\xb3\x82\xea\x82\xbd\x95\xb6\x8e\x9a\x97\xf1\x82\xc5\x82\xb7\x81\x42";
	std::cout
		<< encoding_fixer::guessAndConvertToUtf8(sample_text_sjis) << std::endl;

	std::cout << std::endl;
}
