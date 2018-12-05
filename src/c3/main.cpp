
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

const char * chrfind(const char * str, char ch) {
	while(*str != '\0') {
		if(*str == ch) return str;
		++str;
	}
	return nullptr;
}
const char * strfind1(const char * str, const char * substr, size_t len) {
	while(*str != '\0') {
		const char * str_pos = str;
		const char * substr_pos = substr;
		size_t flen = 0;
		while(*str_pos != '\0' && *str_pos == *substr_pos) {
			++str_pos;
			++substr_pos;
			++flen;
		}
		if(substr_pos == '\0' || flen >= len) {
			return str;
		}
		++str;
	}
	return nullptr;
}

using ltc_ret = std::pair<size_t, std::pair<const char *, const char *>>;
ltc_ret ltc(const char * str, const char * substr, size_t len) {
	if(len == 0) return std::make_pair(0, std::make_pair(nullptr, nullptr));
	if(len == 1) {
		const char * r = chrfind(str, *substr);
		if(r) return std::make_pair(1, std::make_pair(str, substr));
		else return std::make_pair(0, std::make_pair(nullptr, nullptr));
	}
	
	const char * r = strfind1(str, substr, len);
	if(r) {
		return std::make_pair(len, std::make_pair(r, substr));
	}
	else {
		size_t part = len / 2;
		ltc_ret r1 = ltc(str, substr, part);
		ltc_ret r2 = ltc(str, substr + part, len - part);
		if(r1.first) {
			const char * tmp = r1.second.first;
			const char * sub_tmp = r1.second.second;
			size_t ret_len = 0;
			while(*tmp != 0 && *tmp == *sub_tmp) {
				++tmp;
				++sub_tmp;
				++ret_len;
			}
			r1.first = ret_len;
		}
		if(r1.first >= r2.first) {
			return r1;
		}
		else return r2;
	}
	return std::make_pair(0, std::make_pair(nullptr, nullptr));
}

size_t longestCommonSubs(const std::string & str1, const std::string & str2) {
	size_t len = 0;
	const char * str = nullptr;
	const char * substr = nullptr;
	if(str1.length() >= str2.length()) {
		len = str2.length();
		str = str1.c_str();
		substr = str2.c_str();
	}
	else {
		len = str1.length();
		str = str2.c_str();
		substr = str1.c_str();
	}
	
	ltc_ret r = ltc(str, substr, len);
	return r.first;
}





int main(int argc, char ** argv) {
	std::string str1;
	std::string str2;
	
	std::getline(std::cin, str1);
	std::getline(std::cin, str2);
	
	//std::cout << strfind1("aaaabac", "ab") << std::endl;
	std::cout << longestCommonSubs(str1, str2) << std::endl;
	return 0;
}



