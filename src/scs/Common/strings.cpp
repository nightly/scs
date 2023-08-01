#include "scs/Common/strings.h"

#include <span>
#include <string>
#include <sstream>
#include <algorithm>

namespace scs {


	/*
	 * @brief Transforms a vector of strings into a singular state string (_,_,_,_)
	 */
	std::string VectorToString(const std::vector<std::string>& vec) {
		std::string ret;
		for (size_t i = 0; i < vec.size(); i++) {
			ret += vec[i];
			if (i != vec.size() - 1) {
				ret += ',';
			}
		}
		return ret;
	}

	/*
	 * @brief Transforms (_,_,_) into a vector
	 * @param delimiter: default argument of ','
	 */
	std::vector<std::string> StringToVector(const std::string& str, char delimiter) {
		std::vector<std::string> vec;
		std::stringstream stream(str);
		std::string cur;
		while (std::getline(stream, cur, delimiter)) {
			vec.emplace_back(cur);
		}
		return vec;
	}

	/*
	 * @param delimiter: default = ','
	 */
	std::string USetToString(const std::unordered_set<std::string>& set, char delimiter) {
		std::string ret;
		for (auto it = set.begin(); it != set.end(); ++it) {
			if (it != set.begin()) {
				ret += delimiter;
			}
			ret += *it;
		}
		return ret;
	}

	std::string BoolToString(bool b) {
		std::stringstream converter;
		converter << std::boolalpha << b;
		return converter.str();
	}

	std::string& RightTrim(std::string& s, const char* ws) {
		s.erase(s.find_last_not_of(ws) + 1);
		return s;
	}

	std::string& LeftTrim(std::string& s, const char* ws) {
		s.erase(0, s.find_first_not_of(ws));
		return s;
	}

	std::string& Trim(std::string& s, const char* ws) {
		return LeftTrim(RightTrim(s, ws), ws);
	}

	std::string_view Trim(std::string_view view) {
		const auto first = std::find_if_not(view.begin(), view.end(), isspace);
		const auto last = std::find_if_not(view.rbegin(), view.rend(), isspace).base();
		return { first, last };
	}

	bool IsBlank(const char ch) {
		if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\f' || ch == '\v') {
			return true;
		}
		return false;
	}

	bool IsBlank(const std::string_view ch) {
		if (ch == " " || ch == "\n" || ch == "\t" || ch == "\r" || ch == "\f" || ch == "\v") {
			return true;
		}
		return false;
	}

	// Note: only handles ASCII encodings
	std::string ToLower(std::string_view view) {
		std::string ret = view.data();
		std::transform(ret.begin(), ret.end(), ret.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return ret;
	}
}