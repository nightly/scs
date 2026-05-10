#pragma once

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "scs/FirstOrderLogic/fol.h"
#include "scs/SituationCalculus/bat.h"

namespace scs {

	enum class ValidationMode {
		Local,
		Global
	};

	enum class ScDiagnosticSeverity {
		Warning,
		Error
	};

	struct ScParserDiagnostic {
		ScDiagnosticSeverity severity = ScDiagnosticSeverity::Warning;
		std::string message;
		size_t line = 0;
		size_t column = 0;
	};

	struct ParseContext {
		std::unordered_set<std::string> objects;
		std::unordered_set<std::string> variables;
	};

	struct ScParseOptions {
		ValidationMode validation = ValidationMode::Local;
		bool validate = true;
	};

	class SituationCalculusParser {
	public:
		SituationCalculusParser(std::string_view source, ScParseOptions options = {});

		Formula ParseFormula(const ParseContext& context = {});
		BasicActionTheory ParseBasicActionTheory();

		const std::vector<ScParserDiagnostic>& Diagnostics() const;
	private:
		std::string source_;
		ScParseOptions options_;
		std::vector<ScParserDiagnostic> diagnostics_;
	};

	Formula ParseScFormula(std::string_view source, const ParseContext& context = {});
	BasicActionTheory ParseBasicActionTheory(std::string_view source, ScParseOptions options = {});
	void ValidateBasicActionTheory(const BasicActionTheory& bat, ValidationMode mode);

}
