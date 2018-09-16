#include <errno.h>
#include <iconv.h>
#include <string>
#include <memory>
#include <algorithm>

namespace encoding_fixer{

namespace{
	const iconv_t iconv_err = iconv_t(-1);
	const size_t auto_detect = size_t(-1);
}

enum class Encoding {
	sjis,
	euc_jp,
	utf8,
	utf16,
	size
};

const char* const getEncChar(Encoding enc){
	switch(enc){
		case Encoding::sjis:   return "Shift-JIS";
		case Encoding::euc_jp: return "EUC-JP";
		case Encoding::utf8:   return "UTF-8";
		case Encoding::utf16:  return "UTF-16";
		default: return "";
	}
}

std::string convertToUtf8(Encoding enc, char *src, size_t src_size){
	iconv_t iconv_handler = iconv_open(getEncChar(Encoding::utf8), getEncChar(enc));
	if (iconv_handler == iconv_err)
		throw std::runtime_error("unable to get iconv handler");
	std::shared_ptr<void> defer(nullptr, [&](...){
		iconv_close(iconv_handler);
	});

	size_t dst_size = src_size*8;
	// 0初期化
	std::unique_ptr<char[]> dst(new char[dst_size]());
	char *dst_p = dst.get();

	size_t iconv_return = iconv(iconv_handler, &src, &src_size, &dst_p, &dst_size);
	if (iconv_return == -1)
		throw std::runtime_error("conversion error");

	return std::string(dst.get());
}

}	// ~ namespace encoding_fixer

