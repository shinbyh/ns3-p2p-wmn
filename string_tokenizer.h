/*
 * string_tokenizer.h
 *
 *  Created on: Oct 20, 2016
 *  Reference: https://wiki.kldp.org/HOWTO/html/C++Programming-HOWTO/standard-string.html
 */

#ifndef SRC_UTIL_STRING_TOKENIZER_H_
#define SRC_UTIL_STRING_TOKENIZER_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

namespace std {

inline void tokenizeString(const string& str,
		vector<string>& tokens,
		const string& delimiters = " ") {
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos) {
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

} /* namespace std */

#endif /* SRC_UTIL_STRING_TOKENIZER_H_ */
