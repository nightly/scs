#pragma once

#include <string>
#include <span>
#include <vector>
#include <unordered_set>
#include <string_view>

namespace scs {

	std::string VectorToString(const std::vector<std::string>& vector);
	std::vector<std::string> StringToVector(const std::string& str, char delimiter = ',');

	std::string USetToString(const std::unordered_set<std::string>& set, char delimiter = ',');

	std::string BoolToString(bool b);

	constexpr char const* blank = " \n\t\r\f\v";
	std::string& RightTrim(std::string& s, const char* t = blank);
	std::string& LeftTrim(std::string& s, const char* t = blank);
	std::string& Trim(std::string& s, const char* t = blank);
	std::string_view Trim(std::string_view view);

	bool IsBlank(const char ch);
	bool IsBlank(const std::string_view ch);

	std::string ToLower(std::string_view view);

}