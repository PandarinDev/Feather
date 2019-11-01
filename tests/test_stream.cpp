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
	std::string foo("foo");
	std::string bar("bar");
	REQUIRE(Stream({ foo, bar }).sum() == "foobar");
}

TEST_CASE("Stream should be collectible") {
	REQUIRE(Stream({ 1, 2, 3 }).collect<std::vector<int>>() == std::vector<int> { 1, 2, 3 });
}

TEST_CASE("Stream should be countable") {
	REQUIRE(Stream({ 1, 2, 3 }).count() == 3);
	REQUIRE(Stream({ 2, 4, 6, 8, 10 }).count() == 5);
}

TEST_CASE("Stream should be able to tell if any of the elements match a given predicate") {
	auto is_even = [](const auto& i) { return i % 2 == 0; };
	REQUIRE(Stream({ 1, 3, 5, 7, 9 }).any_match(is_even) == false);
	REQUIRE(Stream({ 1, 3, 6, 7, 9 }).any_match(is_even) == true);
}

TEST_CASE("Stream should be able to tell if all of the elements match a given predicate") {
	auto is_odd = [](const auto& i) { return i % 2 == 1; };
	REQUIRE(Stream({ 1, 3, 5, 7, 9 }).all_match(is_odd) == true);
	REQUIRE(Stream({ 1, 3, 6, 7, 9 }).all_match(is_odd) == false);
}
