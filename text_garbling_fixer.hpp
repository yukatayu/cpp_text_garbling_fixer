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

Encoding guessEncoding(const std::string& text){
	std::array<unsigned, static_cast<size_t>(Encoding::size)> err_table;

	auto try_decode = [&](Encoding enc) {
		iconv_t iconv_handler;

		// UTF-32へ変換を試みる(UCS-4の3つのうちどれが有効なのかは環境依存)
		for (const char* iconv_encode : {"UCS-4-INTERNAL", "UCS-4", "UCS-4-SWAPPED"}) {
			iconv_handler = iconv_open(iconv_encode, getEncChar(enc));
			if (iconv_handler != iconv_err)
				break;
		}
		if (iconv_handler == iconv_err)
			throw std::runtime_error("Cannot handle utf32");

		auto& error_count = err_table[static_cast<int>(enc)];

		size_t src_size = text.size();
		size_t dst_size = src_size*8;

		char* src = const_cast<char*>(text.data());
		std::unique_ptr<char[]> dst(new char[dst_size]());
		char *dst_p = dst.get();

		char* src_cpy = src;
		char* dst_cpy = dst_p;

		// カウントはiconvが制御する
		while (0 < src_size) {
			size_t iconv_return = iconv(iconv_handler, &src_cpy, &src_size, &dst_cpy, &dst_size);
			if (errno == EILSEQ) { //無効なバイトシーケンス(signal定数)
				++error_count;
				++src_cpy;
				--src_size;
				continue;
			}
			if (iconv_return == -1)
				break;	// 読み込み失敗
		}
		iconv_close(iconv_handler);
	};

	for (int enc = 0; enc != static_cast<int>(Encoding::size); ++enc) {
		if(enc == static_cast<int>(Encoding::utf16))
			continue;
		err_table[enc] = 0;
		try_decode(static_cast<Encoding>(enc));
	}

	return static_cast<Encoding>(
		std::min_element(err_table.begin(), err_table.end(), [](unsigned a, unsigned b) {
			return a < b;
		}) - err_table.begin()
	);
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

std::string convertUnicode(std::string text) {
	using namespace std;
	for(auto&& esc : {"¥U+"s, "\\U+"s, "¥u+"s, "\\u+"s}){
		const size_t esc_length = esc.size();

		for (size_t done_pos = 0; true; ++done_pos) {
			size_t pos = text.find(esc, done_pos);
			if (pos == std::string::npos)
				break;

			//現状、\\U+4文字のみ対応
			std::string target = text.substr(pos + esc_length, 4);
			if (target.size() < 4)
				break;

			char target_u16[2];
			try {
				target_u16[1] = std::stoi(target.substr(0, 2), nullptr, 16);
				target_u16[0] = std::stoi(target.substr(2, 2), nullptr, 16);
			} catch (std::invalid_argument& e) {
				throw std::runtime_error("invalid U+.... string");
			}
			text.replace(pos, esc_length + 4,  convertToUtf8(Encoding::utf16, target_u16, 2) );
		}
	}
	return text;
}

}	// ~ namespace encoding_fixer

