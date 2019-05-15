#include "stream.h"

#include <catch2/catch.hpp>

using namespace feather;

TEST_CASE("Stream should be constructible") {
	auto stream = Stream({ 1, 2, 3 });
	REQUIRE(stream.next() == 1);
	REQUIRE(stream.next() == 2);
	REQUIRE(stream.next() == 3);
	REQUIRE(stream.next() == std::nullopt);
}

TEST_CASE("Stream should be filterable") {
	auto stream = Stream({ 1, 2, 3, 4, 5, 6 })
		.filter([](const auto& e) { return e % 2 == 0; });
	REQUIRE(stream.next() == 2);
	REQUIRE(stream.next() == 4);
	REQUIRE(stream.next() == 6);
	REQUIRE(stream.next() == std::nullopt);
}

TEST_CASE("Stream should be mappable") {
	auto stream = Stream({ 1, 2, 3 })
		.map([](const auto& e) { return e * e; });
	REQUIRE(stream.next() == 1);
	REQUIRE(stream.next() == 4);
	REQUIRE(stream.next() == 9);
	REQUIRE(stream.next() == std::nullopt);
}

TEST_CASE("Stream should be reducable") {
	auto result = Stream({ 2, 3, 4 })
		.reduce(1, [](const auto& s, const auto& e) { return s * e; });
	REQUIRE(result == 24);
}

TEST_CASE("Stream should be summarizable if it has addition operator") {
	REQUIRE(Stream({ 1, 2, 3 }).sum() == 6);
}
