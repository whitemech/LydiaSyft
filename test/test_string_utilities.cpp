#include <catch2/catch_test_macros.hpp>

#include "string_utilities.h"


TEST_CASE("Split function is tested", "[split]") {
  SECTION("Splitting with default space delimiter") {
    std::string test_string = "Hello World Test";
    std::vector<std::string> result = Syft::split(test_string);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "Hello");
    REQUIRE(result[1] == "World");
    REQUIRE(result[2] == "Test");
  }

  SECTION("Splitting with ':' delimiter") {
    std::string test_string = "Hello:World:Test";
    std::vector<std::string> result = Syft::split(test_string, ":");
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "Hello");
    REQUIRE(result[1] == "World");
    REQUIRE(result[2] == "Test");
  }

  SECTION("Splitting with 'AB' delimiter") {
    std::string test_string = "HelloABWorldABTest";
    std::vector<std::string> result = Syft::split(test_string, "AB");
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "Hello");
    REQUIRE(result[1] == "World");
    REQUIRE(result[2] == "Test");
  }

  SECTION("Splitting with no delimiter in string") {
    std::string test_string = "HelloWorldTest";
    std::vector<std::string> result = Syft::split(test_string, ":");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "HelloWorldTest");
  }

  SECTION("Splitting empty string") {
    std::string test_string = "";
    std::vector<std::string> result = Syft::split(test_string, ":");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "");
  }

  SECTION("Splitting string with delimiter only") {
    std::string test_string = ":::";
    std::vector<std::string> result = Syft::split(test_string, ":");
    REQUIRE(result.size() == 4);
    for (const auto& str : result) {
      REQUIRE(str == "");
    }
  }

  SECTION("Splitting string with multiple consecutive delimiters") {
    std::string test_string = "Hello::World::Test";
    std::vector<std::string> result = Syft::split(test_string, ":");
    REQUIRE(result.size() == 5);
    REQUIRE(result[0] == "Hello");
    REQUIRE(result[1] == "");
    REQUIRE(result[2] == "World");
    REQUIRE(result[3] == "");
    REQUIRE(result[4] == "Test");
  }

  SECTION("Splitting string with leading and trailing delimiters") {
    std::string test_string = ":Hello:World:Test:";
    std::vector<std::string> result = Syft::split(test_string, ":");
    REQUIRE(result.size() == 5);
    REQUIRE(result[0] == "");
    REQUIRE(result[1] == "Hello");
    REQUIRE(result[2] == "World");
    REQUIRE(result[3] == "Test");
    REQUIRE(result[4] == "");
  }

  SECTION("Splitting string where string equals delimiter") {
    std::string test_string = "::";
    std::vector<std::string> result = Syft::split(test_string, "::");
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == "");
    REQUIRE(result[1] == "");
  }
}


TEST_CASE( "Trim function is tested", "[trim]" ) {
  SECTION("Test with leading and trailing spaces") {
    std::string test_string = "  Hello World  ";
    std::string result = Syft::trim(test_string);
    REQUIRE(result == "Hello World");
  }

  SECTION("Test with leading spaces") {
    std::string test_string = "  Hello World";
    std::string result = Syft::trim(test_string);
    REQUIRE(result == "Hello World");
  }

  SECTION("Test with trailing spaces") {
    std::string test_string = "Hello World  ";
    std::string result = Syft::trim(test_string);
    REQUIRE(result == "Hello World");
  }

  SECTION("Test with no extra spaces") {
    std::string test_string = "Hello World";
    std::string result = Syft::trim(test_string);
    REQUIRE(result == "Hello World");
  }

  SECTION("Test with empty string") {
    std::string test_string = "";
    std::string result = Syft::trim(test_string);
    REQUIRE(result == "");
  }

  SECTION("Test with spaces only") {
    std::string test_string = "     ";
    std::string result = Syft::trim(test_string);
    REQUIRE(result == "");
  }

  SECTION("Test with tab characters") {
    std::string test_string = "\tHello World\t";
    std::string result = Syft::trim(test_string);
    REQUIRE(result == "Hello World");
  }
}
