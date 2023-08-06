#include <gtest/gtest.h>
#include "scs/ConGolog/CharacteristicGraph/characteristic_graph.h"

#include <format>
#include "Hinge/resource_1.h"
#include "Hinge/resource_2.h"
#include "Hinge/resource_3.h"
#include "Hinge/resource_4.h"
#include "Hinge/Full/recipe.h"
#include "scs/Common/io.h"

using namespace scs;
using namespace scs::examples;

static std::string export_dir = "../../tests/ConGolog/CharacteristicGraph/Exports/Hinge/";

static std::string ExportDirResource(size_t i) {
	return std::format("../../tests/ConGolog/CharacteristicGraph/Exports/Hinge/Resource{}.gv", i);
}

static std::string TestDirResource(size_t i) {
	return std::format("../../tests/ConGolog/CharacteristicGraph/testdata/Hinge/Resource{}.gv", i);
}

TEST(CgGenerate, HingeResource1) {
	auto Resource1 = scs::examples::HingeResource1();
	CharacteristicGraph cg(Resource1.program, ProgramType::Resource);
	ExportGraph(cg, "Resource1", export_dir);
	scs::PrintCurrentDir();

	EXPECT_TRUE(AreFilesEqual(ExportDirResource(1), TestDirResource(1)));
}

TEST(CgGenerate, HingeResource2) {
	auto Resource2 = scs::examples::HingeResource2();
	CharacteristicGraph cg(Resource2.program, ProgramType::Resource);
	ExportGraph(cg, "Resource2", export_dir);

	EXPECT_TRUE(AreFilesEqual(ExportDirResource(2), TestDirResource(2)));
}

TEST(CgGenerate, HingeResource3) {
	auto Resource3 = scs::examples::HingeResource3();
	CharacteristicGraph cg(Resource3.program, ProgramType::Resource);
	ExportGraph(cg, "Resource3", export_dir);

	EXPECT_TRUE(AreFilesEqual(ExportDirResource(3), TestDirResource(3)));

}

TEST(CgGenerate, HingeResource4) {
	auto Resource4 = scs::examples::HingeResource4();
	CharacteristicGraph cg(Resource4.program, ProgramType::Resource);
	ExportGraph(cg, "Resource4", export_dir);

	EXPECT_TRUE(AreFilesEqual(ExportDirResource(4), TestDirResource(4)));

}

TEST(CgGenerate, HingeRecipe) {
	auto Recipe = scs::examples::HingeRecipe(2);
	CharacteristicGraph cg(Recipe, ProgramType::Recipe);
	ExportGraph(cg, "Recipe", export_dir);

	std::string export_recipe_path = "../../tests/ConGolog/CharacteristicGraph/Exports/Hinge/Recipe.gv";
	std::string test_recipe_path = "../../tests/ConGolog/CharacteristicGraph/testdata/Hinge/Recipe.gv";

	EXPECT_TRUE(AreFilesEqual(export_recipe_path, test_recipe_path));
}