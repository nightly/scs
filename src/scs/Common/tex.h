#pragma once

#include <string>
#include <string_view>

namespace scs {

	inline std::string EscapeTex(std::string_view value) {
		std::string result;
		result.reserve(value.size());
		for (const char character : value) {
			switch (character) {
			case '\\': result += "\\textbackslash{}"; break;
			case '{': result += "\\{"; break;
			case '}': result += "\\}"; break;
			case '$': result += "\\$"; break;
			case '&': result += "\\&"; break;
			case '#': result += "\\#"; break;
			case '_': result += "\\_"; break;
			case '%': result += "\\%"; break;
			case '~': result += "\\textasciitilde{}"; break;
			case '^': result += "\\textasciicircum{}"; break;
			case '\n': result += " "; break;
			default: result += character; break;
			}
		}
		return result;
	}

}
