#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_all.hpp"
#include "integration_tests_utils.hpp"
#include <iostream>

namespace Syft{
namespace Test{


TEST_CASE("Test GFand patterns", "[integration][gfand]") {
  auto problem = GENERATE(GeneratorWrapper<problem_t>(
      Catch::Detail::make_unique<GFAndDatasetProblemGenerator>()));
  check_realizability(problem);
}

TEST_CASE("Test Uright patterns", "[integration][uright]") {
  auto problem = GENERATE(GeneratorWrapper<problem_t>(
      Catch::Detail::make_unique<URightDatasetProblemGenerator>()));
  check_realizability(problem);
}

TEST_CASE("Test Single-counter", "[integration][single_counter]") {
  // we take only the first three instances, the next ones are too complex for
  // Nike
  auto problem = GENERATE(
      take(5, GeneratorWrapper<problem_t>(
                  Catch::Detail::make_unique<SingleCounterDatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Double-counter", "[integration][double_counter]") {
  // we take only the first two instances, the next ones are too complex for
  // Nike
  auto problem = GENERATE(
      take(3, GeneratorWrapper<problem_t>(
                   Catch::Detail::make_unique<DoubleCounterDatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Nim-1", "[integration][nim][nim_1]") {
  auto problem =
      GENERATE(take(5, GeneratorWrapper<problem_t>(
                           Catch::Detail::make_unique<Nim1DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Nim-2", "[integration][nim][nim_2]") {
  auto problem =
      GENERATE(take(2, GeneratorWrapper<problem_t>(
                           Catch::Detail::make_unique<Nim2DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Nim-3", "[integration][nim][nim_3]") {
  // we take only the first instance, the next ones are too complex for Nike
  auto problem =
      GENERATE(take(1, GeneratorWrapper<problem_t>(
                           Catch::Detail::make_unique<Nim3DatasetProblemGenerator>())));
  check_realizability(problem);
}

TEST_CASE("Test Lydia Random case 03 50",
          "[integration][lydia_random][lydia_random_03_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_03_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_03_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Lydia Random case 04 50",
          "[integration][lydia_random][lydia_random_04_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_04_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_04_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Lydia Random case 05 50",
          "[integration][lydia_random][lydia_random_05_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_05_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_05_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Lydia Random case 06 50",
          "[integration][lydia_random][lydia_random_06_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_06_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_06_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Lydia Random case 07 50",
          "[integration][lydia_random][lydia_random_07_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_07_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_07_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Lydia Random case 08 50",
          "[integration][lydia_random][lydia_random_08_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_08_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_08_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Lydia Random case 09 50",
          "[integration][lydia_random][lydia_random_09_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_09_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_09_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Lydia Random case 10 50",
          "[integration][lydia_random][lydia_random_10_50]") {
  auto problem = GENERATE(filter(
      tractable_lydia_random_10_50,
      GeneratorWrapper<problem_t>(
          Catch::Detail::make_unique<LydiaRandom_10_50_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Syft Random case 1",
          "[integration][syft_random][syft_random_1]") {
  auto problem = GENERATE(
      filter(tractable_syft_random_1,
             GeneratorWrapper<problem_t>(
                 Catch::Detail::make_unique<SyftRandom_1_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Syft Random case 2",
          "[integration][syft_random][syft_random_2]") {
  auto problem = GENERATE(
      filter(tractable_syft_random_2,
             GeneratorWrapper<problem_t>(
                 Catch::Detail::make_unique<SyftRandom_2_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Syft Random case 3",
          "[integration][syft_random][syft_random_3]") {
  auto problem = GENERATE(
      filter(tractable_syft_random_3,
             GeneratorWrapper<problem_t>(
                 Catch::Detail::make_unique<SyftRandom_3_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Syft Random case 4",
          "[integration][syft_random][syft_random_4]") {
  auto problem = GENERATE(
      filter(tractable_syft_random_4,
             GeneratorWrapper<problem_t>(
                 Catch::Detail::make_unique<SyftRandom_4_DatasetProblemGenerator>())));
  check_realizability(problem);
}
TEST_CASE("Test Syft Random case 5",
          "[integration][syft_random][syft_random_5]") {
  auto problem = GENERATE(
      filter(tractable_syft_random_5,
             GeneratorWrapper<problem_t>(
                 Catch::Detail::make_unique<SyftRandom_5_DatasetProblemGenerator>())));
  check_realizability(problem);
}

}
}