#include <gtest/gtest.h>
#include "scs/Combinatorics/CartesianProduct/product.h"

using namespace scs;

TEST(ComProduct, Int) {
	std::vector v1 = { 10, 20, 5 };
	std::vector v2 = { 2, 10 };
	std::vector v3 = { 9 };
	std::vector v4 = { 50, 25, 100 };
	std::vector v5 = { 1000, 2000, 3000, 4000, 5000, 6000 };
	std::vector<const std::vector<int>*> vecs = { &v1, &v2, &v3, &v4, &v5 };

	auto x = Product(vecs);
	ASSERT_EQ(x.size(), 108);
}

TEST(ComProduct, String) {
	std::vector<std::string> v1 = { "v1a", "v1b", "v1c" };
	std::vector<std::string> v2 = { "v2a", "v2b" };
	std::vector<std::string> v3 = { "v3a", "v3b", "v3c" };
	std::vector<std::string> v4 = { "v4a" };
	std::vector<std::string> v5 = { "v5a", "v5b", "v5c", "v5d", "v5e", "v5f" };
	std::vector<std::string> v6 = { "v6a", "v6b", "v6c" };
	std::vector<const std::vector<std::string>*> vecs = { &v1, &v2, &v3, &v4, &v5, &v6 };

	auto product = Product(vecs);
	ASSERT_EQ(product.size(), 324);
}

TEST(ComProduct, EmptyVec) {
	std::vector<int> v0{ };
	std::vector<int> v1{ 2, 10 };
	std::vector<int> v2{ };
	std::vector<int> v3{ 50, 25 };
	std::vector<const std::vector<int>*> vecs = { &v0, &v1, &v2, &v3 };
	
	auto product = Product(vecs);
	ASSERT_EQ(product.size(), 4);
	std::cout << product;
}

TEST(ComProduct, EmptyVecWithFlagValue) {
	std::vector<std::string> v1 = { "v1a", "v1b", "v1c" };
	std::vector<std::string> v2 = { "v2a", "v2b" };
	std::vector<std::string> v3 = { };
	std::vector<std::string> v4 = { "v3a", "v3b", "v3c" };
	std::vector<const std::vector<std::string>*> vecs = { &v1, &v2, &v3, &v4 };

	auto product = Product(vecs, std::string("missing"));
	ASSERT_EQ(product.size(), 18);
	// std::cout << product;
}

