#include "stream.h"

#include <catch2/catch.hpp>
#include <vector>

using namespace feather;

TEST_CASE("Stream should be constructible") {
	std::vector<int> foo { 1, 2, 3 };
	auto stream = Stream(foo);
	REQUIRE(stream.next() == 1);
	REQUIRE(stream.next() == 2);
	REQUIRE(stream.next() == 3);
}

TEST_CASE("Stream should be filterable") {
	std::vector<int> foo { 1, 2, 3, 4, 5, 6 };
	auto stream = Stream(foo).filter([](const auto& e) { return e % 2 == 0; });
	REQUIRE(stream.next() == 2);
	REQUIRE(stream.next() == 4);
	REQUIRE(stream.next() == 6);
}

TEST_CASE("Stream should be mappable") {
	std::vector<int> foo { 1, 2, 3 };
	auto stream = Stream(foo).map([](const auto& e) { return e * e; });
	REQUIRE(stream.next() == 1);
	REQUIRE(stream.next() == 4);
	REQUIRE(stream.next() == 9);
}

TEST_CASE("Stream should be reducable") {
	std::vector<int> foo { 2, 3, 4 };
	auto result = Stream(foo).reduce(1, [](const auto& s, const auto& e) { return s * e; });
	REQUIRE(result == 24);
}

TEST_CASE("Integer stream should be summarizable") {
	std::vector<int> foo { 1, 2, 3 };
	REQUIRE(Stream(foo).sum() == 6);
}
