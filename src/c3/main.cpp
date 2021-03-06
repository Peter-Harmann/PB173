
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
		if(*substr_pos == '\0' || flen >= len) {
			return str;
		}
		++str;
	}
	return nullptr;
}

using ltc_ret = std::pair<size_t, std::pair<const char *, const char *>>;

void refit(ltc_ret & r1, const char * str, const char * substr) {
	const char * tmp = r1.second.first;
	const char * sub_tmp = r1.second.second;
			
	do {
		--tmp;
		--sub_tmp;
	}
	while(substr <= sub_tmp && str <= tmp && *tmp == *sub_tmp);
			
	++tmp;
	++sub_tmp;
	
	r1.second.first = tmp;
	r1.second.second = sub_tmp;
			
	size_t ret_len = 0;
	while(*tmp != 0 && *tmp == *sub_tmp) {
		++tmp;
		++sub_tmp;
		++ret_len;
	}
	r1.first = ret_len;
}


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
	
	//ltc_ret r = ltc(str, substr, len);
	
	size_t part = len;
	while(part >= 1) {
		size_t i = 0;
		ltc_ret r2 = std::make_pair(0, std::make_pair(nullptr, nullptr));
		for(; i<len; i+=part) {
			ltc_ret r1 = ltc(str, substr+i, part);
			if(r1.first) refit(r1, str, substr);
			if(r1.first > r2.first) r1 = r2;
		}
		ltc_ret r1 = ltc(str, substr+i, len - i);
		if(r1.first > r2.first) r1 = r2;
		if(r1.first) {
			return r1.first;
		}
		part /= 2;
	}
	
	return 0;
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



