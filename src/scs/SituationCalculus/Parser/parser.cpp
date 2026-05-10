#include "scs/SituationCalculus/Parser/parser.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

#include "scs/SituationCalculus/action_type.h"

namespace scs {
namespace {

	enum class TokenKind {
		Identifier,
		LParen,
		RParen,
		Comma,
		Dot,
		Equal,
		NotEqual,
		And,
		Or,
		Not,
		Implies,
		Equiv,
		Newline,
		End,
		Objects,
		Type,
		Init,
		Poss,
		Ssa,
		True,
		False,
		Forall,
		Exists
	};

	struct Token {
		TokenKind kind = TokenKind::End;
		std::string text;
		size_t line = 1;
		size_t column = 1;
	};

	std::string ToLower(std::string_view view) {
		std::string ret(view);
		std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
		});
		return ret;
	}

	bool StartsWith(std::string_view source, size_t pos, std::string_view prefix) {
		return pos + prefix.size() <= source.size() && source.substr(pos, prefix.size()) == prefix;
	}

	bool IsAtomStart(unsigned char c) {
		return std::isalnum(c) || c == '_';
	}

	bool IsAtomPart(unsigned char c) {
		return std::isalnum(c) || c == '_' || c == '\'';
	}

	class Lexer {
	public:
		explicit Lexer(std::string_view source) : source_(source) {}

		Token Next() {
			SkipHorizontalWhitespaceAndComments();
			if (pos_ >= source_.size()) {
				return Make(TokenKind::End, "");
			}

			const size_t token_line = line_;
			const size_t token_col = column_;

			if (source_[pos_] == '\n') {
				AdvanceByte();
				return {TokenKind::Newline, "\n", token_line, token_col};
			}

			if (StartsWith(source_, pos_, "&&")) {
				AdvanceBytes(2);
				return {TokenKind::And, "&&", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "||")) {
				AdvanceBytes(2);
				return {TokenKind::Or, "||", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "!=")) {
				AdvanceBytes(2);
				return {TokenKind::NotEqual, "!=", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "==")) {
				AdvanceBytes(2);
				return {TokenKind::Equal, "==", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "->")) {
				AdvanceBytes(2);
				return {TokenKind::Implies, "->", token_line, token_col};
			}

			if (StartsWith(source_, pos_, "∧")) {
				AdvanceBytes(std::string_view("∧").size());
				return {TokenKind::And, "∧", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "∨")) {
				AdvanceBytes(std::string_view("∨").size());
				return {TokenKind::Or, "∨", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "¬")) {
				AdvanceBytes(std::string_view("¬").size());
				return {TokenKind::Not, "¬", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "∀")) {
				AdvanceBytes(std::string_view("∀").size());
				return {TokenKind::Forall, "∀", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "∃")) {
				AdvanceBytes(std::string_view("∃").size());
				return {TokenKind::Exists, "∃", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "⊃")) {
				AdvanceBytes(std::string_view("⊃").size());
				return {TokenKind::Implies, "⊃", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "≡")) {
				AdvanceBytes(std::string_view("≡").size());
				return {TokenKind::Equiv, "≡", token_line, token_col};
			}
			if (StartsWith(source_, pos_, "≠")) {
				AdvanceBytes(std::string_view("≠").size());
				return {TokenKind::NotEqual, "≠", token_line, token_col};
			}

			const char c = source_[pos_];
			switch (c) {
			case '(':
				AdvanceByte();
				return {TokenKind::LParen, "(", token_line, token_col};
			case ')':
				AdvanceByte();
				return {TokenKind::RParen, ")", token_line, token_col};
			case ',':
				AdvanceByte();
				return {TokenKind::Comma, ",", token_line, token_col};
			case '.':
				AdvanceByte();
				return {TokenKind::Dot, ".", token_line, token_col};
			case '=':
				AdvanceByte();
				return {TokenKind::Equal, "=", token_line, token_col};
			case '^':
				AdvanceByte();
				return {TokenKind::And, "^", token_line, token_col};
			case '!':
				AdvanceByte();
				return {TokenKind::Not, "!", token_line, token_col};
			default:
				break;
			}

			if (IsAtomStart(static_cast<unsigned char>(c))) {
				return Identifier();
			}

			throw Error(std::format("unexpected character '{}'", c), token_line, token_col);
		}
	private:
		std::string_view source_;
		size_t pos_ = 0;
		size_t line_ = 1;
		size_t column_ = 1;

		Token Make(TokenKind kind, std::string text) const {
			return {kind, std::move(text), line_, column_};
		}

		void AdvanceByte() {
			if (pos_ >= source_.size()) {
				return;
			}
			if (source_[pos_] == '\n') {
				++line_;
				column_ = 1;
			} else {
				++column_;
			}
			++pos_;
		}

		void AdvanceBytes(size_t count) {
			for (size_t i = 0; i < count; ++i) {
				AdvanceByte();
			}
		}

		void SkipHorizontalWhitespaceAndComments() {
			while (pos_ < source_.size()) {
				const char c = source_[pos_];
				if (c == ' ' || c == '\t' || c == '\r') {
					AdvanceByte();
					continue;
				}
				if (c == '%' || c == '#') {
					while (pos_ < source_.size() && source_[pos_] != '\n') {
						AdvanceByte();
					}
					continue;
				}
				if (StartsWith(source_, pos_, "//")) {
					while (pos_ < source_.size() && source_[pos_] != '\n') {
						AdvanceByte();
					}
					continue;
				}
				break;
			}
		}

		Token Identifier() {
			const size_t start = pos_;
			const size_t token_line = line_;
			const size_t token_col = column_;
			while (pos_ < source_.size()) {
				const auto c = static_cast<unsigned char>(source_[pos_]);
				if (IsAtomPart(c)) {
					AdvanceByte();
				} else if (source_[pos_] == '.' && pos_ > start && pos_ + 1 < source_.size() &&
					std::isdigit(static_cast<unsigned char>(source_[pos_ - 1])) &&
					std::isdigit(static_cast<unsigned char>(source_[pos_ + 1]))) {
					AdvanceByte();
				} else {
					break;
				}
			}

			std::string text(source_.substr(start, pos_ - start));
			const std::string lower = ToLower(text);
			if (lower == "objects") return {TokenKind::Objects, text, token_line, token_col};
			if (lower == "type") return {TokenKind::Type, text, token_line, token_col};
			if (lower == "init") return {TokenKind::Init, text, token_line, token_col};
			if (lower == "poss") return {TokenKind::Poss, text, token_line, token_col};
			if (lower == "ssa") return {TokenKind::Ssa, text, token_line, token_col};
			if (lower == "true") return {TokenKind::True, text, token_line, token_col};
			if (lower == "false") return {TokenKind::False, text, token_line, token_col};
			if (lower == "and") return {TokenKind::And, text, token_line, token_col};
			if (lower == "or") return {TokenKind::Or, text, token_line, token_col};
			if (lower == "not") return {TokenKind::Not, text, token_line, token_col};
			if (lower == "implies") return {TokenKind::Implies, text, token_line, token_col};
			if (lower == "equiv") return {TokenKind::Equiv, text, token_line, token_col};
			if (lower == "forall") return {TokenKind::Forall, text, token_line, token_col};
			if (lower == "exists") return {TokenKind::Exists, text, token_line, token_col};
			return {TokenKind::Identifier, text, token_line, token_col};
		}

		std::runtime_error Error(const std::string& message, size_t line, size_t column) const {
			return std::runtime_error(std::format("[SituationCalculusParser] {} at {}:{}", message, line, column));
		}
	};

	struct Ast;
	using AstPtr = std::unique_ptr<Ast>;

	struct AstBool {
		bool value = false;
	};

	struct AstSymbol {
		std::string name;
	};

	struct AstCall {
		std::string name;
		std::vector<AstPtr> args;
	};

	struct AstUnary {
		UnaryKind kind = UnaryKind::Negation;
		AstPtr child;
	};

	struct AstBinary {
		BinaryKind kind = BinaryKind::Conjunction;
		AstPtr lhs;
		AstPtr rhs;
	};

	struct AstQuantifier {
		QuantifierKind kind = QuantifierKind::Universal;
		std::vector<std::string> variables;
		AstPtr child;
	};

	struct Ast {
		std::variant<AstBool, AstSymbol, AstCall, AstUnary, AstBinary, AstQuantifier> node;
		size_t line = 0;
		size_t column = 0;
	};

	AstPtr MakeAst(auto node, const Token& token) {
		auto ret = std::make_unique<Ast>();
		ret->node = std::move(node);
		ret->line = token.line;
		ret->column = token.column;
		return ret;
	}

	struct FormulaReferences {
		std::vector<std::pair<std::string, size_t>> predicates;
		std::vector<std::pair<std::string, size_t>> actions;
		std::vector<std::string> objects;
	};

	void ValidateBasicActionTheoryImpl(const BasicActionTheory& bat, ValidationMode mode,
		std::vector<ScParserDiagnostic>* diagnostics, std::vector<std::string>* errors);

	bool IsActionVariableAst(const Ast& ast) {
		if (const auto* sym = std::get_if<AstSymbol>(&ast.node)) {
			return sym->name == "a";
		}
		return false;
	}

	bool IsReservedVariableName(std::string_view name) {
		return name == "a" || name == "cv";
	}

	bool IsValidVariableName(std::string_view name) {
		if (name.empty()) {
			return false;
		}
		return !std::isdigit(static_cast<unsigned char>(name.front()));
	}

	class InternalParser {
	public:
		InternalParser(std::string_view source, ScParseOptions options, std::vector<ScParserDiagnostic>& diagnostics)
			: lexer_(source), options_(options), diagnostics_(diagnostics) {
			Next();
		}

		Formula ParseFormula(const ParseContext& context) {
			context_ = context;
			ValidateContext(context_);
			SkipNewlines();
			AstPtr ast = ParseExpression(1);
			if (current_.kind != TokenKind::End && current_.kind != TokenKind::Newline) {
				Fail(std::format("unexpected token '{}'", current_.text));
			}
			return ResolveFormula(*ast, context_);
		}

		BasicActionTheory ParseBasicActionTheory() {
			BasicActionTheory bat;
			bat.is_global = false;
			Situation initial;

			SkipNewlines();
			while (current_.kind != TokenKind::End) {
				switch (current_.kind) {
				case TokenKind::Objects:
					ParseObjects(bat);
					break;
				case TokenKind::Type:
					ParseType(bat);
					break;
				case TokenKind::Init:
					ParseInit(initial, bat);
					break;
				case TokenKind::Poss:
					ParsePoss(bat);
					break;
				case TokenKind::Ssa:
					ParseSsa(bat);
					break;
				default:
					Fail(std::format("expected BAT statement, got '{}'", current_.text));
				}
				ConsumeStatementEnd();
			}

			bat.SetInitial(std::move(initial));
			if (options_.validate) {
				ValidateWithDiagnostics(bat, options_.validation);
			}
			return bat;
		}
	private:
		Lexer lexer_;
		Token current_;
		ScParseOptions options_;
		std::vector<ScParserDiagnostic>& diagnostics_;
		ParseContext context_;

		void Next() {
			current_ = lexer_.Next();
		}

		bool Check(TokenKind kind) const {
			return current_.kind == kind;
		}

		Token Consume(TokenKind kind, std::string_view expected) {
			if (!Check(kind)) {
				Fail(std::format("expected {}, got '{}'", expected, current_.text));
			}
			Token ret = current_;
			Next();
			return ret;
		}

		void Fail(const std::string& message) const {
			throw std::runtime_error(std::format("[SituationCalculusParser] {} at {}:{}", message, current_.line, current_.column));
		}

		void Warn(const std::string& message, const Token& token) {
			diagnostics_.push_back({ScDiagnosticSeverity::Warning, message, token.line, token.column});
		}

		void ValidateContext(const ParseContext& context) const {
			for (const std::string& variable : context.variables) {
				if (!IsValidVariableName(variable)) {
					throw std::runtime_error(std::format("[SituationCalculusParser] invalid variable name '{}'", variable));
				}
				if (context.objects.contains(variable) && !IsReservedVariableName(variable)) {
					throw std::runtime_error(std::format("[SituationCalculusParser] symbol '{}' is both an object and a variable", variable));
				}
			}
		}

		void SkipNewlines() {
			while (Check(TokenKind::Newline)) {
				Next();
			}
		}

		void ConsumeStatementEnd() {
			if (Check(TokenKind::End)) {
				return;
			}
			Consume(TokenKind::Newline, "end of line");
			SkipNewlines();
		}

		static int Precedence(TokenKind kind) {
			switch (kind) {
			case TokenKind::Equiv:
				return 1;
			case TokenKind::Implies:
				return 2;
			case TokenKind::Or:
				return 3;
			case TokenKind::And:
				return 4;
			case TokenKind::Equal:
			case TokenKind::NotEqual:
				return 5;
			default:
				return 0;
			}
		}

		static bool IsRightAssociative(TokenKind kind) {
			return kind == TokenKind::Implies || kind == TokenKind::Equiv;
		}

		static BinaryKind ToBinaryKind(TokenKind kind) {
			switch (kind) {
			case TokenKind::And:
				return BinaryKind::Conjunction;
			case TokenKind::Or:
				return BinaryKind::Disjunction;
			case TokenKind::Implies:
				return BinaryKind::Implication;
			case TokenKind::Equiv:
				return BinaryKind::Equivalence;
			case TokenKind::Equal:
				return BinaryKind::Equal;
			case TokenKind::NotEqual:
				return BinaryKind::NotEqual;
			default:
				throw std::logic_error("token is not a binary operator");
			}
		}

		AstPtr ParseExpression(int min_precedence) {
			AstPtr lhs = ParsePrefix();
			while (true) {
				const int precedence = Precedence(current_.kind);
				if (precedence < min_precedence) {
					break;
				}
				Token op = current_;
				Next();
				const int rhs_min = precedence + (IsRightAssociative(op.kind) ? 0 : 1);
				AstPtr rhs = ParseExpression(rhs_min);
				lhs = MakeAst(AstBinary{ToBinaryKind(op.kind), std::move(lhs), std::move(rhs)}, op);
			}
			return lhs;
		}

		AstPtr ParsePrefix() {
			if (Check(TokenKind::Not)) {
				Token token = current_;
				Next();
				return MakeAst(AstUnary{UnaryKind::Negation, ParseExpression(6)}, token);
			}
			if (Check(TokenKind::Forall) || Check(TokenKind::Exists)) {
				return ParseQuantifier();
			}
			return ParsePrimary();
		}

		AstPtr ParseQuantifier() {
			Token token = current_;
			const QuantifierKind kind = Check(TokenKind::Forall) ? QuantifierKind::Universal : QuantifierKind::Existential;
			Next();

			std::vector<std::string> variables;
			Token variable = Consume(TokenKind::Identifier, "quantified variable");
			ValidateVariableToken(variable, nullptr);
			if (context_.objects.contains(variable.text)) {
				throw std::runtime_error(std::format("[SituationCalculusParser] quantified variable '{}' conflicts with a known object at {}:{}",
					variable.text, variable.line, variable.column));
			}
			variables.push_back(variable.text);
			while (Check(TokenKind::Comma)) {
				Next();
				variable = Consume(TokenKind::Identifier, "quantified variable");
				ValidateVariableToken(variable, nullptr);
				if (context_.objects.contains(variable.text)) {
					throw std::runtime_error(std::format("[SituationCalculusParser] quantified variable '{}' conflicts with a known object at {}:{}",
						variable.text, variable.line, variable.column));
				}
				if (std::find(variables.begin(), variables.end(), variable.text) != variables.end()) {
					throw std::runtime_error(std::format("[SituationCalculusParser] duplicate quantified variable '{}' at {}:{}",
						variable.text, variable.line, variable.column));
				}
				variables.push_back(variable.text);
			}
			Consume(TokenKind::Dot, "'.' after quantified variables");
			return MakeAst(AstQuantifier{kind, std::move(variables), ParseExpression(1)}, token);
		}

		AstPtr ParsePrimary() {
			if (Check(TokenKind::True)) {
				Token token = current_;
				Next();
				return MakeAst(AstBool{true}, token);
			}
			if (Check(TokenKind::False)) {
				Token token = current_;
				Next();
				return MakeAst(AstBool{false}, token);
			}
			if (Check(TokenKind::Identifier)) {
				Token token = current_;
				Next();
				if (Check(TokenKind::LParen)) {
					return ParseCall(token);
				}
				return MakeAst(AstSymbol{token.text}, token);
			}
			if (Check(TokenKind::LParen)) {
				Next();
				AstPtr child = ParseExpression(1);
				Consume(TokenKind::RParen, "')'");
				return child;
			}
			Fail(std::format("expected expression, got '{}'", current_.text));
			return nullptr;
		}

		AstPtr ParseCall(const Token& name) {
			Consume(TokenKind::LParen, "'('");
			std::vector<AstPtr> args;
			if (!Check(TokenKind::RParen)) {
				args.push_back(ParseExpression(1));
				while (Check(TokenKind::Comma)) {
					Next();
					args.push_back(ParseExpression(1));
				}
			}
			Consume(TokenKind::RParen, "')'");
			return MakeAst(AstCall{name.text, std::move(args)}, name);
		}

		void ValidateVariableToken(const Token& token, const BasicActionTheory* bat) const {
			if (!IsValidVariableName(token.text)) {
				throw std::runtime_error(std::format("[SituationCalculusParser] invalid variable name '{}' at {}:{}",
					token.text, token.line, token.column));
			}
			if (IsReservedVariableName(token.text)) {
				throw std::runtime_error(std::format("[SituationCalculusParser] '{}' is reserved at {}:{}",
					token.text, token.line, token.column));
			}
			if (bat && bat->objects.contains(Object{token.text})) {
				throw std::runtime_error(std::format("[SituationCalculusParser] formal variable '{}' conflicts with a declared object at {}:{}",
					token.text, token.line, token.column));
			}
		}

		std::vector<Term> ParseFormalParameters(const BasicActionTheory* bat) {
			std::vector<Term> terms;
			if (!Check(TokenKind::LParen)) {
				return terms;
			}
			Next();
			if (!Check(TokenKind::RParen)) {
				Token param = Consume(TokenKind::Identifier, "formal parameter");
				ValidateVariableToken(param, bat);
				terms.emplace_back(Variable{param.text});
				while (Check(TokenKind::Comma)) {
					Next();
					param = Consume(TokenKind::Identifier, "formal parameter");
					ValidateVariableToken(param, bat);
					if (std::find_if(terms.begin(), terms.end(), [&param](const Term& term) {
						const auto* variable = std::get_if<Variable>(&term);
						return variable && variable->name() == param.text;
					}) != terms.end()) {
						throw std::runtime_error(std::format("[SituationCalculusParser] duplicate formal parameter '{}' at {}:{}",
							param.text, param.line, param.column));
					}
					terms.emplace_back(Variable{param.text});
				}
			}
			Consume(TokenKind::RParen, "')'");
			return terms;
		}

		std::pair<std::string, std::vector<Term>> ParseSignature(const BasicActionTheory* bat) {
			const std::string name = Consume(TokenKind::Identifier, "name").text;
			return {name, ParseFormalParameters(bat)};
		}

		std::vector<Object> ParseObjectArguments(const BasicActionTheory& bat) {
			std::vector<Object> objects;
			if (!Check(TokenKind::LParen)) {
				return objects;
			}
			Next();
			if (!Check(TokenKind::RParen)) {
				objects.push_back(ParseObjectArgument(bat));
				while (Check(TokenKind::Comma)) {
					Next();
					objects.push_back(ParseObjectArgument(bat));
				}
			}
			Consume(TokenKind::RParen, "')'");
			return objects;
		}

		Object ParseObjectArgument(const BasicActionTheory& bat) {
			Token token = Consume(TokenKind::Identifier, "object");
			if (!bat.objects.contains(Object{token.text})) {
				Warn(std::format("object '{}' used in initial valuation but not declared in this BAT", token.text), token);
			}
			return Object{token.text};
		}

		void ParseObjects(BasicActionTheory& bat) {
			Next();
			ParseObjectDeclaration(bat);
			while (Check(TokenKind::Comma)) {
				Next();
				ParseObjectDeclaration(bat);
			}
		}

		void ParseObjectDeclaration(BasicActionTheory& bat) {
			Token object = Consume(TokenKind::Identifier, "object");
			if (IsReservedVariableName(object.text)) {
				throw std::runtime_error(std::format("[SituationCalculusParser] object '{}' conflicts with a reserved symbol at {}:{}",
					object.text, object.line, object.column));
			}
			if (bat.objects.contains(Object{object.text})) {
				throw std::runtime_error(std::format("[SituationCalculusParser] duplicate object '{}' at {}:{}",
					object.text, object.line, object.column));
			}
			bat.objects.emplace(object.text);
		}

		static ActionType ParseActionTypeValue(const Token& token) {
			const std::string value = ToLower(token.text);
			if (value == "manufacturing") {
				return ActionType::Manufacturing;
			}
			if (value == "nop") {
				return ActionType::Nop;
			}
			if (value == "transfer") {
				return ActionType::Transfer;
			}
			if (value == "preparatory") {
				return ActionType::Preparatory;
			}
			throw std::runtime_error(std::format("[SituationCalculusParser] unknown action type '{}' at {}:{}",
				token.text, token.line, token.column));
		}

		void ParseType(BasicActionTheory& bat) {
			Next();
			Token action_name = Consume(TokenKind::Identifier, "action name");
			Consume(TokenKind::Equal, "'='");
			Token type_name = Consume(TokenKind::Identifier, "action type");
			bat.types[action_name.text] = ParseActionTypeValue(type_name);
		}

		void ParseInit(Situation& initial, const BasicActionTheory& bat) {
			Next();
			Token name = Consume(TokenKind::Identifier, "fluent name");
			std::vector<Object> args = ParseObjectArguments(bat);
			Consume(TokenKind::Equal, "'='");
			bool value = false;
			if (Check(TokenKind::True)) {
				value = true;
			} else if (Check(TokenKind::False)) {
				value = false;
			} else {
				Fail("expected true or false for initial valuation");
			}
			Next();

			auto& fluent = initial.relational_fluents_[name.text];
			if (args.empty()) {
				fluent.AddValuation(value);
			} else {
				fluent.AddValuation(std::move(args), value);
			}
		}

		ParseContext ContextForTerms(const std::vector<Term>& terms, const BasicActionTheory& bat) const {
			ParseContext context;
			for (const auto& object : bat.objects) {
				context.objects.insert(object.name());
			}
			for (const Term& term : terms) {
				if (const auto* var = std::get_if<Variable>(&term)) {
					context.variables.insert(var->name());
				}
			}
			context.variables.insert("a");
			context.variables.insert("cv");
			return context;
		}

		void ParsePoss(BasicActionTheory& bat) {
			Next();
			auto [name, terms] = ParseSignature(&bat);
			if (bat.pre.contains(name) && bat.pre.at(name).Terms().size() != terms.size()) {
				Fail(std::format("conflicting arity for Poss '{}'", name));
			}
			Consume(TokenKind::Equal, "'='");
			context_ = ContextForTerms(terms, bat);
			AstPtr formula_ast = ParseExpression(1);
			bat.pre[name] = Poss{std::move(terms), ResolveFormula(*formula_ast, context_)};
		}

		void ParseSsa(BasicActionTheory& bat) {
			Next();
			auto [name, terms] = ParseSignature(&bat);
			if (bat.successors.contains(name) && bat.successors.at(name).Terms().size() != terms.size()) {
				Fail(std::format("conflicting arity for SSA '{}'", name));
			}
			Consume(TokenKind::Equal, "'='");
			context_ = ContextForTerms(terms, bat);
			AstPtr formula_ast = ParseExpression(1);
			bat.successors[name] = Successor{std::move(terms), ResolveFormula(*formula_ast, context_)};
		}

		Formula ResolveFormula(const Ast& ast, ParseContext context) {
			if (const auto* b = std::get_if<AstBool>(&ast.node)) {
				return b->value;
			}
			if (const auto* sym = std::get_if<AstSymbol>(&ast.node)) {
				return ResolveSymbol(*sym, context);
			}
			if (const auto* call = std::get_if<AstCall>(&ast.node)) {
				return ResolveCallAsFormula(*call, context, ast);
			}
			if (const auto* unary = std::get_if<AstUnary>(&ast.node)) {
				return Box<UnaryConnective>{new UnaryConnective{ResolveFormula(*unary->child, context), unary->kind}};
			}
			if (const auto* binary = std::get_if<AstBinary>(&ast.node)) {
				const bool action_comparison = (binary->kind == BinaryKind::Equal || binary->kind == BinaryKind::NotEqual) &&
					(IsActionVariableAst(*binary->lhs) || IsActionVariableAst(*binary->rhs));
				Formula lhs = action_comparison ? ResolveActionSide(*binary->lhs, context) : ResolveFormula(*binary->lhs, context);
				Formula rhs = action_comparison ? ResolveActionSide(*binary->rhs, context) : ResolveFormula(*binary->rhs, context);
				return Box<BinaryConnective>{new BinaryConnective{lhs, rhs, binary->kind}};
			}
			if (const auto* quantifier = std::get_if<AstQuantifier>(&ast.node)) {
				ParseContext child_context = context;
				for (const std::string& var : quantifier->variables) {
					child_context.variables.insert(var);
				}
				Formula child = ResolveFormula(*quantifier->child, child_context);
				for (auto it = quantifier->variables.rbegin(); it != quantifier->variables.rend(); ++it) {
					child = Box<Quantifier>{new Quantifier{Variable{*it}, child, quantifier->kind}};
				}
				return child;
			}
			throw std::logic_error("unhandled AST node");
		}

		Formula ResolveSymbol(const AstSymbol& symbol, const ParseContext& context) const {
			if (symbol.name == "cv") {
				return Variable{"cv"};
			}
			if (context.objects.contains(symbol.name) && !context.variables.contains(symbol.name)) {
				return Object{symbol.name};
			}
			return Variable{symbol.name};
		}

		static Formula TermToFormula(const Term& term) {
			if (const auto* object = std::get_if<Object>(&term)) {
				return *object;
			}
			return std::get<Variable>(term);
		}

		Term ResolveTerm(const Ast& ast, const ParseContext& context) const {
			if (const auto* sym = std::get_if<AstSymbol>(&ast.node)) {
				if (context.variables.contains(sym->name) || !context.objects.contains(sym->name)) {
					return Variable{sym->name};
				}
				return Object{sym->name};
			}
			if (const auto* call = std::get_if<AstCall>(&ast.node)) {
				const std::string lower = ToLower(call->name);
				if (lower == "obj" && call->args.size() == 1) {
					if (const auto* sym = std::get_if<AstSymbol>(&call->args[0]->node)) {
						return Object{sym->name};
					}
				}
				if (lower == "var" && call->args.size() == 1) {
					if (const auto* sym = std::get_if<AstSymbol>(&call->args[0]->node)) {
						return Variable{sym->name};
					}
				}
			}
			throw std::runtime_error(std::format("[SituationCalculusParser] expected term at {}:{}", ast.line, ast.column));
		}

		Action ResolveActionCall(const AstCall& call, const ParseContext& context) const {
			std::vector<Term> terms;
			for (const AstPtr& arg : call.args) {
				terms.push_back(ResolveTerm(*arg, context));
			}
			return Action{call.name, std::move(terms)};
		}

		Formula ResolveActionSide(const Ast& ast, const ParseContext& context) {
			if (const auto* call = std::get_if<AstCall>(&ast.node)) {
				const std::string lower = ToLower(call->name);
				if (lower == "obj" || lower == "var") {
					return TermToFormula(ResolveTerm(ast, context));
				}
				if (lower == "act") {
					return ResolveExplicitAction(*call, context, ast);
				}
				return ResolveActionCall(*call, context);
			}
			return ResolveFormula(ast, context);
		}

		Formula ResolveExplicitAction(const AstCall& call, const ParseContext& context, const Ast& ast) const {
			if (call.args.size() != 1) {
				throw std::runtime_error(std::format("[SituationCalculusParser] act(...) expects one action call at {}:{}",
					ast.line, ast.column));
			}
			const auto* action_call = std::get_if<AstCall>(&call.args[0]->node);
			if (!action_call) {
				throw std::runtime_error(std::format("[SituationCalculusParser] act(...) expects one action call at {}:{}",
					call.args[0]->line, call.args[0]->column));
			}
			return ResolveActionCall(*action_call, context);
		}

		Formula ResolveExplicitPredicate(const AstCall& call, const ParseContext& context, const Ast& ast) const {
			if (call.args.size() != 1) {
				throw std::runtime_error(std::format("[SituationCalculusParser] pred(...) expects one predicate call at {}:{}",
					ast.line, ast.column));
			}
			const auto* pred_call = std::get_if<AstCall>(&call.args[0]->node);
			if (!pred_call) {
				throw std::runtime_error(std::format("[SituationCalculusParser] pred(...) expects one predicate call at {}:{}",
					call.args[0]->line, call.args[0]->column));
			}
			std::vector<Term> terms;
			for (const AstPtr& arg : pred_call->args) {
				terms.push_back(ResolveTerm(*arg, context));
			}
			return Predicate{pred_call->name, std::move(terms)};
		}

		Formula ResolveCallAsFormula(const AstCall& call, const ParseContext& context, const Ast& ast) {
			const std::string lower = ToLower(call.name);
			if (lower == "obj" || lower == "var") {
				return TermToFormula(ResolveTerm(ast, context));
			}
			if (lower == "act") {
				return ResolveExplicitAction(call, context, ast);
			}
			if (lower == "pred") {
				return ResolveExplicitPredicate(call, context, ast);
			}
			if (lower == "coop") {
				if (call.args.size() != 2) {
					throw std::runtime_error(std::format("[SituationCalculusParser] coop(...) expects two variables at {}:{}",
						ast.line, ast.column));
				}
				const auto lhs = ResolveTerm(*call.args[0], context);
				const auto rhs = ResolveTerm(*call.args[1], context);
				const auto* lhs_var = std::get_if<Variable>(&lhs);
				const auto* rhs_var = std::get_if<Variable>(&rhs);
				if (!lhs_var || !rhs_var) {
					throw std::runtime_error(std::format("[SituationCalculusParser] coop(...) arguments must be variables at {}:{}",
						ast.line, ast.column));
				}
				return CoopPredicate{*lhs_var, *rhs_var};
			}
			if (lower == "route") {
				if (call.args.size() != 2) {
					throw std::runtime_error(std::format("[SituationCalculusParser] route(...) expects two object indices at {}:{}",
						ast.line, ast.column));
				}
				const auto lhs = ResolveTerm(*call.args[0], context);
				const auto rhs = ResolveTerm(*call.args[1], context);
				const auto* lhs_obj = std::get_if<Object>(&lhs);
				const auto* rhs_obj = std::get_if<Object>(&rhs);
				if (!lhs_obj || !rhs_obj) {
					throw std::runtime_error(std::format("[SituationCalculusParser] route(...) arguments must be objects at {}:{}",
						ast.line, ast.column));
				}
				return RoutePredicate{std::stoull(lhs_obj->name()), std::stoull(rhs_obj->name())};
			}

			std::vector<Term> terms;
			for (const AstPtr& arg : call.args) {
				terms.push_back(ResolveTerm(*arg, context));
			}
			return Predicate{call.name, std::move(terms)};
		}

		static void CollectRefs(const Formula& formula, FormulaReferences& refs) {
			std::visit([&refs](const auto& value) {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, Predicate>) {
					refs.predicates.emplace_back(value.name(), value.terms().size());
					for (const Term& term : value.terms()) {
						if (const auto* object = std::get_if<Object>(&term)) {
							refs.objects.push_back(object->name());
						}
					}
				} else if constexpr (std::is_same_v<T, Action>) {
					refs.actions.emplace_back(value.name, value.terms.size());
					for (const Term& term : value.terms) {
						if (const auto* object = std::get_if<Object>(&term)) {
							refs.objects.push_back(object->name());
						}
					}
				} else if constexpr (std::is_same_v<T, Object>) {
					refs.objects.push_back(value.name());
				} else if constexpr (std::is_same_v<T, Box<UnaryConnective>>) {
					CollectRefs(value->child(), refs);
				} else if constexpr (std::is_same_v<T, Box<BinaryConnective>>) {
					CollectRefs(value->lhs(), refs);
					CollectRefs(value->rhs(), refs);
				} else if constexpr (std::is_same_v<T, Box<Quantifier>>) {
					CollectRefs(value->child(), refs);
				}
			}, formula);
		}

		void ValidateWithDiagnostics(const BasicActionTheory& bat, ValidationMode mode) {
			std::vector<std::string> errors;
			ValidateBasicActionTheoryImpl(bat, mode, &diagnostics_, &errors);
			if (!errors.empty()) {
				std::ostringstream ss;
				ss << "[SituationCalculusParser] validation failed:";
				for (const std::string& error : errors) {
					ss << "\n- " << error;
				}
				throw std::runtime_error(ss.str());
			}
		}
	};

	void AddFinding(ValidationMode mode, std::vector<ScParserDiagnostic>* diagnostics, std::vector<std::string>* errors,
		const std::string& message) {
		if (mode == ValidationMode::Global) {
			if (errors) {
				errors->push_back(message);
			}
			return;
		}
		if (diagnostics) {
			diagnostics->push_back({ScDiagnosticSeverity::Warning, message, 0, 0});
		}
	}

	void CollectFormulaRefs(const Formula& formula, FormulaReferences& refs) {
		std::visit([&refs](const auto& value) {
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, Predicate>) {
				refs.predicates.emplace_back(value.name(), value.terms().size());
				for (const Term& term : value.terms()) {
					if (const auto* object = std::get_if<Object>(&term)) {
						refs.objects.push_back(object->name());
					}
				}
			} else if constexpr (std::is_same_v<T, Action>) {
				refs.actions.emplace_back(value.name, value.terms.size());
				for (const Term& term : value.terms) {
					if (const auto* object = std::get_if<Object>(&term)) {
						refs.objects.push_back(object->name());
					}
				}
			} else if constexpr (std::is_same_v<T, Object>) {
				refs.objects.push_back(value.name());
			} else if constexpr (std::is_same_v<T, Box<UnaryConnective>>) {
				CollectFormulaRefs(value->child(), refs);
			} else if constexpr (std::is_same_v<T, Box<BinaryConnective>>) {
				CollectFormulaRefs(value->lhs(), refs);
				CollectFormulaRefs(value->rhs(), refs);
			} else if constexpr (std::is_same_v<T, Box<Quantifier>>) {
				CollectFormulaRefs(value->child(), refs);
			}
		}, formula);
	}

	void ValidateBasicActionTheoryImpl(const BasicActionTheory& bat, ValidationMode mode,
		std::vector<ScParserDiagnostic>* diagnostics, std::vector<std::string>* errors) {
		std::unordered_map<std::string, size_t> fluent_arities;
		for (const auto& [name, fluent] : bat.Initial().Fluents()) {
			fluent_arities[name] = fluent.Arity();
		}

		std::unordered_map<std::string, size_t> action_arities;
		for (const auto& [name, poss] : bat.pre) {
			action_arities[name] = poss.Terms().size();
		}

		auto validate_object = [&](const std::string& name) {
			if (!bat.objects.contains(Object{name})) {
				AddFinding(mode, diagnostics, errors,
					std::format("object '{}' is referenced but is not declared in the BAT object domain", name));
			}
		};

		auto validate_predicate = [&](const std::string& name, size_t arity) {
			const auto fluent_it = fluent_arities.find(name);
			if (fluent_it == fluent_arities.end()) {
				AddFinding(mode, diagnostics, errors,
					std::format("fluent '{}' is referenced but has no initial valuation", name));
				return;
			}
			if (fluent_it->second != 8080 && fluent_it->second != arity) {
				AddFinding(mode, diagnostics, errors,
					std::format("fluent '{}' is referenced with arity {}, but initial valuation arity is {}",
						name, arity, fluent_it->second));
			}
		};

		auto validate_formula = [&](const Formula& formula) {
			FormulaReferences refs;
			CollectFormulaRefs(formula, refs);
			for (const auto& [name, arity] : refs.predicates) {
				validate_predicate(name, arity);
			}
			for (const auto& [name, arity] : refs.actions) {
				const auto action_it = action_arities.find(name);
				if (action_it != action_arities.end() && action_it->second != arity) {
					AddFinding(mode, diagnostics, errors,
						std::format("action '{}' is referenced with arity {}, but Poss arity is {}",
							name, arity, action_it->second));
				}
			}
			for (const std::string& object : refs.objects) {
				validate_object(object);
			}
		};

		for (const auto& [name, poss] : bat.pre) {
			validate_formula(poss.Form());
		}
		for (const auto& [name, successor] : bat.successors) {
			validate_predicate(name, successor.Terms().size());
			validate_formula(successor.Form());
		}
	}

}

	SituationCalculusParser::SituationCalculusParser(std::string_view source, ScParseOptions options)
		: source_(source), options_(options) {}

	Formula SituationCalculusParser::ParseFormula(const ParseContext& context) {
		InternalParser parser(source_, options_, diagnostics_);
		return parser.ParseFormula(context);
	}

	BasicActionTheory SituationCalculusParser::ParseBasicActionTheory() {
		InternalParser parser(source_, options_, diagnostics_);
		return parser.ParseBasicActionTheory();
	}

	const std::vector<ScParserDiagnostic>& SituationCalculusParser::Diagnostics() const {
		return diagnostics_;
	}

	Formula ParseScFormula(std::string_view source, const ParseContext& context) {
		SituationCalculusParser parser(source);
		return parser.ParseFormula(context);
	}

	BasicActionTheory ParseBasicActionTheory(std::string_view source, ScParseOptions options) {
		SituationCalculusParser parser(source, options);
		return parser.ParseBasicActionTheory();
	}

	void ValidateBasicActionTheory(const BasicActionTheory& bat, ValidationMode mode) {
		std::vector<std::string> errors;
		ValidateBasicActionTheoryImpl(bat, mode, nullptr, &errors);
		if (!errors.empty()) {
			std::ostringstream ss;
			ss << "[SituationCalculusParser] validation failed:";
			for (const std::string& error : errors) {
				ss << "\n- " << error;
			}
			throw std::runtime_error(ss.str());
		}
	}

}
