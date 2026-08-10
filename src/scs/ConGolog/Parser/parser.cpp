#include "scs/ConGolog/Parser/parser.h"

#include <algorithm>
#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <magic_enum/magic_enum.hpp>

#include "scs/ConGolog/Program/programs.h"
#include "scs/FirstOrderLogic/formula_utils.h"
#include "scs/SituationCalculus/Parser/parser.h"

namespace scs {
namespace {

	void CollectFormulaObjects(const Formula& formula, std::vector<Object>& objects) {
		std::visit([&](const auto& value) {
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, Object>) {
				objects.push_back(value);
			} else if constexpr (std::is_same_v<T, Predicate>) {
				for (const Term& term : value.terms()) {
					if (const auto* object = std::get_if<Object>(&term)) objects.push_back(*object);
				}
			} else if constexpr (std::is_same_v<T, Action>) {
				for (const Term& term : value.terms) {
					if (const auto* object = std::get_if<Object>(&term)) objects.push_back(*object);
				}
			} else if constexpr (std::is_same_v<T, CompoundAction>) {
				for (const Action& action : value.Actions()) {
					for (const Term& term : action.terms) {
						if (const auto* object = std::get_if<Object>(&term)) objects.push_back(*object);
					}
				}
			} else if constexpr (std::is_same_v<T, Box<UnaryConnective>>) {
				CollectFormulaObjects(value->child(), objects);
			} else if constexpr (std::is_same_v<T, Box<BinaryConnective>>) {
				CollectFormulaObjects(value->lhs(), objects);
				CollectFormulaObjects(value->rhs(), objects);
			} else if constexpr (std::is_same_v<T, Box<Quantifier>>) {
				CollectFormulaObjects(value->child(), objects);
			}
		}, formula);
	}

	class ProgramParser {
	public:
		ProgramParser(std::string source, const BasicActionTheory& bat)
			: source_(std::move(source)), bat_(bat) {
			Lexer lexer(source_);
			for (Token token : lexer.AllTokens()) {
				if (token.type != TokenType::Newline) tokens_.push_back(std::move(token));
			}
			for (const Object& object : bat_.rigid_objects) rigid_names_.insert(object.name());
		}

		std::shared_ptr<IProgram> Parse() {
			if (Check(TokenType::EndOfFile)) Fail("expected a program");
			auto program = ParseBranch();
			if (!Check(TokenType::EndOfFile)) {
				Fail(std::format("unexpected token '{}'", Current().view));
			}
			return program;
		}
	private:
		std::string source_;
		const BasicActionTheory& bat_;
		std::vector<Token> tokens_;
		size_t position_ = 0;
		std::unordered_map<std::string, size_t> bound_variables_;
		std::unordered_set<std::string> rigid_names_;

		const Token& Current() const { return tokens_.at(position_); }
		bool Check(TokenType type) const { return Current().type == type; }
		Token Consume(TokenType type, std::string_view expected) {
			if (!Check(type)) Fail(std::format("expected {}, got '{}'", expected, Current().view));
			return tokens_.at(position_++);
		}
		bool Accept(TokenType type) {
			if (!Check(type)) return false;
			++position_;
			return true;
		}

		[[noreturn]] void Fail(std::string_view message) const {
			throw std::runtime_error(std::format("[ConGologParser] {} at {}:{}",
				message, Current().line, Current().column));
		}

		std::shared_ptr<IProgram> ParseBranch() {
			auto lhs = ParseInterleaved();
			while (Accept(TokenType::NonDet)) {
				auto rhs = ParseInterleaved();
				lhs = std::make_shared<Branch>(*lhs, *rhs);
			}
			return lhs;
		}

		std::shared_ptr<IProgram> ParseInterleaved() {
			auto lhs = ParseSynchronized();
			while (Accept(TokenType::InterleavedConcurrency)) {
				auto rhs = ParseSynchronized();
				lhs = std::make_shared<Interleaved>(*lhs, *rhs);
			}
			return lhs;
		}

		std::shared_ptr<IProgram> ParseSynchronized() {
			auto lhs = ParseSequence();
			while (Accept(TokenType::SynchronizedConcurrency)) {
				auto rhs = ParseSequence();
				lhs = std::make_shared<Simultaneous>(*lhs, *rhs);
			}
			return lhs;
		}

		std::shared_ptr<IProgram> ParseSequence() {
			auto lhs = ParsePostfix();
			while (Accept(TokenType::SemiColon)) {
				auto rhs = ParsePostfix();
				lhs = std::make_shared<Sequence>(*lhs, *rhs);
			}
			return lhs;
		}

		std::shared_ptr<IProgram> ParsePostfix() {
			auto program = ParsePrimary();
			while (Accept(TokenType::Star)) program = std::make_shared<Iteration>(*program);
			return program;
		}

		bool LooksLikeTest() const {
			size_t depth = 0;
			for (size_t i = position_; i < tokens_.size(); ++i) {
				const TokenType type = tokens_[i].type;
				if (type == TokenType::LParen) {
					++depth;
				} else if (type == TokenType::RParen) {
					if (depth == 0) return false;
					--depth;
				} else if (depth == 0 && type == TokenType::QuestionMark) {
					return true;
				} else if (depth == 0 && (type == TokenType::SemiColon || type == TokenType::NonDet ||
					type == TokenType::InterleavedConcurrency || type == TokenType::SynchronizedConcurrency ||
					type == TokenType::Star || type == TokenType::Else || type == TokenType::EndIf ||
					type == TokenType::Endwhile || type == TokenType::EndLoop || type == TokenType::EndOfFile)) {
					return false;
				}
			}
			return false;
		}

		Formula ParseFormulaUntil(std::initializer_list<TokenType> ends) {
			const size_t start_index = position_;
			size_t depth = 0;
			auto is_end = [&](TokenType type) {
				return std::ranges::find(ends, type) != ends.end();
			};
			while (!Check(TokenType::EndOfFile)) {
				const TokenType type = Current().type;
				if (depth == 0 && is_end(type)) break;
				if (type == TokenType::LParen) {
					++depth;
				} else if (type == TokenType::RParen) {
					if (depth == 0) break;
					--depth;
				}
				++position_;
			}
			if (position_ == start_index) Fail("expected a formula");
			const Token& first = tokens_[start_index];
			const size_t end_offset = Current().offset;
			Formula formula = ParseScFormula(std::string_view(source_).substr(first.offset, end_offset - first.offset),
				FormulaContext());
			for (const Variable& variable : FreeVariables(formula)) {
				if (!bound_variables_.contains(variable.name())) {
					Fail(std::format("variable '{}' must be introduced by pick", variable.name()));
				}
			}
			std::vector<Object> objects;
			CollectFormulaObjects(formula, objects);
			for (const Object& object : objects) {
				if (!rigid_names_.contains(object.name())) {
					Fail(std::format("object constant '{}' is not declared rigid", object.name()));
				}
			}
			return formula;
		}

		ParseContext FormulaContext() const {
			ParseContext context;
			context.objects = rigid_names_;
			for (const auto& [name, depth] : bound_variables_) {
				(void)depth;
				context.variables.insert(name);
			}
			return context;
		}

		std::shared_ptr<IProgram> ParsePrimary() {
			if (Check(TokenType::Nil)) {
				++position_;
				return std::make_shared<Nil>();
			}
			if (Check(TokenType::If)) return ParseIf();
			if (Check(TokenType::While)) return ParseWhile();
			if (Check(TokenType::Loop)) return ParseLoop();
			if (Check(TokenType::Pi)) return ParsePick();
			if (LooksLikeTest()) {
				Formula formula = ParseFormulaUntil({TokenType::QuestionMark});
				Consume(TokenType::QuestionMark, "'?'");
				return std::make_shared<scs::Check>(std::move(formula));
			}
			if (Accept(TokenType::LParen)) {
				auto program = ParseBranch();
				Consume(TokenType::RParen, "')'");
				return program;
			}
			return ParseAction();
		}

		std::shared_ptr<IProgram> ParseAction() {
			Token name = Consume(TokenType::Identifier, "an action name");
			std::vector<Term> terms;
			if (Accept(TokenType::LParen)) {
				if (!Check(TokenType::RParen)) {
					terms.push_back(ParseTerm());
					while (Accept(TokenType::Comma)) terms.push_back(ParseTerm());
				}
				Consume(TokenType::RParen, "')'");
			}
			return std::make_shared<ActionProgram>(Action{std::move(name.view), std::move(terms)});
		}

		Term ParseTerm() {
			if (!Check(TokenType::Identifier) && !Check(TokenType::Number)) {
				Fail(std::format("expected an action argument, got '{}'", Current().view));
			}
			Token token = tokens_[position_++];
			if (bound_variables_.contains(token.view)) return Variable{token.view};
			if (!rigid_names_.contains(token.view)) {
				Fail(std::format("object constant '{}' is not declared rigid", token.view));
			}
			return Object::Rigid(token.view);
		}

		std::shared_ptr<IProgram> ParsePick() {
			Consume(TokenType::Pi, "pick or pi");
			const bool parenthesized = Accept(TokenType::LParen);
			std::vector<Variable> variables;
			std::unordered_set<std::string> local_variables;
			auto add_variable = [&] {
				Token token = Consume(TokenType::Identifier, "a variable name");
				if (!local_variables.emplace(token.view).second) {
					Fail(std::format("variable '{}' is bound twice by the same pick", token.view));
				}
				variables.emplace_back(token.view);
			};
			add_variable();
			while (Accept(TokenType::Comma)) add_variable();
			if (parenthesized) Consume(TokenType::RParen, "')'");
			Consume(TokenType::Dot, "'.'");
			for (const Variable& variable : variables) ++bound_variables_[variable.name()];
			auto body = ParseBranch();
			for (const Variable& variable : variables) {
				auto found = bound_variables_.find(variable.name());
				if (--found->second == 0) bound_variables_.erase(found);
			}
			return std::make_shared<Pick>(std::move(variables), *body);
		}

		std::shared_ptr<IProgram> ParseIf() {
			Consume(TokenType::If, "if");
			Formula condition = ParseFormulaUntil({TokenType::Then});
			Consume(TokenType::Then, "then");
			auto yes = ParseBranch();
			Consume(TokenType::Else, "else");
			auto no = ParseBranch();
			Consume(TokenType::EndIf, "endif");
			return std::make_shared<CgIf>(condition, *yes, *no);
		}

		std::shared_ptr<IProgram> ParseWhile() {
			Consume(TokenType::While, "while");
			Formula condition = ParseFormulaUntil({TokenType::Do});
			Consume(TokenType::Do, "do");
			auto body = ParseBranch();
			Consume(TokenType::Endwhile, "endwhile");
			return std::make_shared<CgWhile>(condition, *body);
		}

		std::shared_ptr<IProgram> ParseLoop() {
			Consume(TokenType::Loop, "loop");
			Accept(TokenType::Colon);
			if (Accept(TokenType::Do)) {
				auto body = ParseBranch();
				Consume(TokenType::EndLoop, "endloop");
				return std::make_shared<Loop>(*body);
			}
			auto body = ParsePostfix();
			return std::make_shared<Loop>(*body);
		}
	};

	std::vector<size_t> DirectiveIndices(const std::vector<Token>& tokens, TokenType type) {
		std::vector<size_t> indices;
		for (size_t i = 0; i < tokens.size(); ++i) {
			if (tokens[i].type == type) indices.push_back(i);
		}
		return indices;
	}

}

	Parser::Parser(Lexer&& lexer) : lexer(std::move(lexer)) {
		Reset();
	}

	void Parser::Reset() {
		lexer.Reset();
		tokens_ = lexer.AllTokens();
		position_ = 0;
	}

	const Token& Parser::CurrentToken() const {
		return tokens_.at(std::min(position_, tokens_.size() - 1));
	}

	const Token& Parser::PeekToken() const {
		return tokens_.at(std::min(position_ + 1, tokens_.size() - 1));
	}

	void Parser::NextToken() {
		if (position_ + 1 < tokens_.size()) ++position_;
	}

	bool Parser::CheckToken(TokenType type) const { return CurrentToken().type == type; }
	bool Parser::CheckPeek(TokenType type) const { return PeekToken().type == type; }

	void Parser::Match(TokenType type) const {
		if (!CheckToken(type)) {
			Abort(std::format("expected {}, got {}", magic_enum::enum_name(type),
				magic_enum::enum_name(CurrentToken().type)));
		}
	}

	void Parser::nl() {
		Match(TokenType::Newline);
		while (CheckToken(TokenType::Newline)) NextToken();
	}

	void Parser::set_sym_relational_fluents(const std::unordered_set<std::string>& fluents) {
		symbols_relational_fluents_ = fluents;
	}

	void Parser::set_sym_actions(const std::unordered_set<std::string>& actions) {
		symbols_actions_ = actions;
	}

	const std::unordered_set<std::string>& Parser::sym_relational_fluents() const {
		return symbols_relational_fluents_;
	}

	const std::unordered_set<std::string>& Parser::sym_actions() const {
		return symbols_actions_;
	}

	Resource Parser::Parse() {
		const auto bat_directives = DirectiveIndices(tokens_, TokenType::DirectiveBAT);
		const auto program_directives = DirectiveIndices(tokens_, TokenType::DirectiveProgram);
		if (program_directives.empty()) Abort("missing required #Program directive");
		if (program_directives.size() != 1) Abort("duplicate #Program directive");
		if (bat_directives.size() > 1) Abort("duplicate #BAT directive");
		const size_t program_index = program_directives.front();
		if (!bat_directives.empty() && bat_directives.front() > program_index) {
			Abort("#BAT must appear before #Program");
		}
		const size_t first_directive = bat_directives.empty() ? program_index : bat_directives.front();
		for (size_t i = 0; i < first_directive; ++i) {
			if (tokens_[i].type != TokenType::Newline) Abort("content is not allowed before the first directive");
		}

		BasicActionTheory bat;
		if (!bat_directives.empty()) {
			const Token& bat_token = tokens_[bat_directives.front()];
			const Token& program_token = tokens_[program_index];
			const size_t begin = bat_token.offset + bat_token.length;
			bat = ParseBasicActionTheory(std::string_view(lexer.Source()).substr(begin, program_token.offset - begin));
		}

		const Token& program_token = tokens_[program_index];
		const size_t program_begin = program_token.offset + program_token.length;
		ProgramParser program_parser(lexer.Source().substr(program_begin), bat);
		return Resource{0, program_parser.Parse(), std::move(bat)};
	}

	[[noreturn]] void Parser::Abort(const std::string& message) const {
		throw std::runtime_error(std::format("[ConGologParser] {} at {}:{}",
			message, CurrentToken().line, CurrentToken().column));
	}

}
