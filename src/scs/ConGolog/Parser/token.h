#pragma once

#include <string_view>
#include <optional>

namespace scs { 

	enum class TokenType {

		/*==============Directives===============*/
		DirectiveBAT = 0,
		DirectiveProgram = 1,
		/* ---------------------------------------*/


		/*===========1 character tokens===========*/
		Identifier = 2,
		Number = 3,
		Newline = 4, // \n
		EndOfFile = 5, // \0
		Colon = 6,

		Dot = 7, // . for quantification
		And = 8, // ∧ or ^
		Or = 9, // ∨
		Negation = 10, // ¬
		Implies = 11, // ⊃
		Equivalence = 12, // ≡
		Universal = 13, // ∀
		Existential = 14, // ∃
		Equal = 15, // =
		NotEqual = 16, // ≠

		LParen = 17,
		RParen = 18,
		Comma = 19,
		/* ---------------------------------------*/


		/*=========1 or 2 character tokens========*/

		/* ---------------------------------------*/


		/*===========2 character tokens===========*/
		
		/* ---------------------------------------*/

		/*==========Keywords/Identifiers==========*/
		//================FOL=====================//
		True = 101,
		False = 102, 

		//==============ConGolog==================//
		Do = 103, 
		Loop = 104,
		While = 105,
		Endwhile = 106,
		If = 107,
		Then = 108,
		Else = 109,
		EndIf = 110,
		SemiColon = 111, // Sequence
		QuestionMark = 112, // Condition test
		Star = 113, // Non-deterministic iteration
		Pi = 114,  // Non-deterministic choice

		/*=========== 1+ ConGolog character tokens==========*/
		NonDet = 130, // Non-deterministic branch |
		InterleavedConcurrency = 131, // ||
		SynchronizedConcurrency = 132, // |||
		/* ----------------------------------------------- */


		//===========Situation Calculus===========//
		Poss = 201,
		S0 = 202,
		/* ---------------------------------------*/

		//===========   FOL Keywords    ===========//
		Obj = 301,
		Var = 302,
		/* ---------------------------------------*/
	};

	struct Token {
	public:
		TokenType type;
		std::string_view view;
		size_t length;
		size_t line;
	public:
		Token();
		Token(TokenType type, std::string_view view, size_t length, size_t line);

		static std::optional<TokenType> StringToToken(std::string_view str);
		static std::optional<TokenType> StringToKeyword(std::string_view str);

		friend std::ostream& operator<<(std::ostream& os, const Token& token);
		std::string ToStr() const;
		bool operator==(const Token& other) const;

	};

}