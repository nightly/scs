#include <gtest/gtest.h>

#include <memory>
#include <filesystem>
#include <stdexcept>

#include "scs/ConGolog/Parser/parser.h"
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"
#include "scs/ConGolog/Program/programs.h"
#include "scs/Common/io.h"

namespace {

	scs::Resource Parse(std::string_view source) {
		return scs::Parser{scs::Lexer{source}}.Parse();
	}

}

TEST(ConGologParser, RequiresOneOrderedProgramDirective) {
	EXPECT_THROW(Parse("Nil"), std::runtime_error);
	EXPECT_THROW(Parse("#Program\nNil\n#Program\nNil"), std::runtime_error);
	EXPECT_THROW(Parse("#Program\nNil\n#BAT\nobjects r"), std::runtime_error);
	EXPECT_THROW(Parse("#BAT\nobjects r\n#BAT\nobjects s\n#Program\nNil"), std::runtime_error);
	EXPECT_NO_THROW(Parse("% comment\n#Program\nNil"));
}

TEST(ConGologParser, ParsesBatRigidRelationsAndStructuredPrograms) {
	const auto resource = Parse(R"(
#BAT
objects fixture
rigid linked(fixture) = true
poss A(x) = true
poss B(x) = true
#Program
pick x. if linked(fixture) then (A(x)* || B(x)) else while false do Nil endwhile endif
)");
	ASSERT_NE(resource.program, nullptr);
	EXPECT_NE(dynamic_cast<scs::Pick*>(resource.program.get()), nullptr);
	ASSERT_TRUE(resource.bat.rigid.contains("linked"));
	EXPECT_TRUE(resource.bat.rigid.at("linked").Valuation({scs::Object::Rigid("fixture")}));
}

TEST(ConGologParser, UsesDocumentedConcurrencyPrecedenceAndSpellings) {
	const auto resource = Parse("#Program\nA || B ||| C");
	const auto* interleaved = dynamic_cast<scs::Interleaved*>(resource.program.get());
	ASSERT_NE(interleaved, nullptr);
	EXPECT_NE(dynamic_cast<scs::ActionProgram*>(interleaved->p.get()), nullptr);
	EXPECT_NE(dynamic_cast<scs::Simultaneous*>(interleaved->q.get()), nullptr);
}

TEST(ConGologParser, RejectsFreeVariablesAndNonRigidConstants) {
	EXPECT_THROW(Parse("#Program\nA(x)"), std::runtime_error);
	EXPECT_THROW(Parse("#Program\nready(x)?"), std::runtime_error);
	EXPECT_NO_THROW(Parse("#Program\npick x. A(x)"));
}

TEST(ConGologParser, NestedPickShadowingRetainsTheOuterBinding) {
	const auto resource = Parse("#Program\npick x. ((pick x. A(x)); B(x))");
	EXPECT_NO_THROW(scs::CharacteristicGraph(resource.program, scs::ProgramType::Resource));
}

TEST(ConGologParser, HandlesIdentifiersNumbersUnicodeAndComments) {
	EXPECT_NO_THROW(Parse(R"(
#BAT
objects fixture_1, part', 0.30
#Program
% leading comment
π x'. (Move(x', fixture_1); Move(x', part')) | Move(x', 0.30) // trailing comment
)") );
}

TEST(ConGologParser, ParsesAssemblyFixtures) {
	const std::filesystem::path directory = std::filesystem::path(__FILE__).parent_path()
		/ "../../../examples/Assembly";
	for (const std::string name : {"recipe.cgl", "resource1.cgl", "resource2.cgl", "resource3.cgl"}) {
		const auto resource = Parse(scs::ReadIntoString(directory / name));
		ASSERT_NE(resource.program, nullptr) << name;
		const scs::CharacteristicGraph graph{resource.program, scs::ProgramType::Resource};
		EXPECT_GT(graph.lts.NumOfStates(), 0) << name;
	}
}
