#pragma once
/*
 * This file is part of Nike.
 *
 * Nike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <filesystem>
#include <set>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "lydia/types.hpp"
#include "InputOutputPartition.h"

#include "utils.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Parser.h"

namespace Syft::Test {

enum SynthesisResult {
  REALIZABLE = 0,
  UNREALIZABLE = 1,
  UNKNOWN = 2,
};

bool test_is_realizable(const whitemech::lydia::ltlf_ptr &formula,
                               const Syft::InputOutputPartition &partition) {
  auto result = Syft::Test::get_realizability(formula, partition);
  return result;
}

// index in the dataset, path to TLSF file, is realizable
typedef std::tuple<size_t, std::filesystem::path, SynthesisResult>problem_t;

void check_realizability(const problem_t &problem) {
  // unpack input
  size_t index;
  std::filesystem::path formula_path;
  SynthesisResult expected_realizability;
  std::tie(index, formula_path, expected_realizability) = problem;

  std::cout << get_time_str() << (": Processing problem " + formula_path.string()) << std::endl;

  // prepare input
  Syft::Parser parser;
  // the parser assumes "syfco" is in the current working directory
  parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, formula_path);
  auto formula_str =  parser.get_formula();

  auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
  auto formula = Syft::Test::parse_formula(formula_str, *driver);

  auto partition = Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(), parser.get_output_variables());

  bool actual_realizability_bool = test_is_realizable(formula, partition);
  SynthesisResult actual_realizability = actual_realizability_bool
                                             ? SynthesisResult::REALIZABLE
                                             : SynthesisResult::UNREALIZABLE;
  std::cout << get_time_str() << ": Result computed." << std::endl;
  REQUIRE(actual_realizability == expected_realizability);
}

bool check_index_in_set(const problem_t &problem, const std::set<size_t> &s) {
  return s.find(std::get<0>(problem) + 1) != s.end();
}

template <typename IsRealizableChecker>
class AbstractDatasetProblemGenerator
    : public Catch::Generators::IGenerator<problem_t> {
private:
  std::filesystem::path dataset_folder_;
  int current_index = 0;
  std::vector<problem_t> problems;

public:
  explicit AbstractDatasetProblemGenerator(
      const std::filesystem::path &dataset_folder)
      : dataset_folder_(dataset_folder), current_index{0} {
    assert(is_directory(dataset_folder));

    std::filesystem::directory_iterator directory_iterator{dataset_folder};
    std::vector<std::filesystem::path> files;
    for (auto &entry : directory_iterator)
      files.push_back(entry);
    std::sort(files.begin(), files.end());
    for (auto i = 0; i < files.size(); ++i) {
      auto formula = files[i];
      problems.emplace_back(i, formula, IsRealizableChecker()(i, formula));
    }
  }

  const problem_t &get() const override {
    return this->problems[this->current_index];
  };
  bool next() override {
    this->current_index += 1;
    return this->current_index < problems.size();
  }
};

static const std::filesystem::path DATASETS_PATH = Syft::Test::DATASET_FOLDER;

// to be used as template argument for AbstractDatasetProblemGenerator
struct always_unrealizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    return SynthesisResult::UNREALIZABLE;
  }
};

// to be used as template argument for AbstractDatasetProblemGenerator
struct always_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    return SynthesisResult::REALIZABLE;
  }
};

//********************************
// GF-and dataset generator
//********************************

class GFAndDatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<always_unrealizable> {
public:
  GFAndDatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH / "Patterns/GFand") {}
};

//********************************
// Uright dataset generator
//********************************

struct is_uright_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    // only first is unrealizable
    if (index == 0)
      return SynthesisResult::UNREALIZABLE;
    // the other instances are realizable
    return SynthesisResult::REALIZABLE;
  }
};

class URightDatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_uright_problem_realizable> {
public:
  URightDatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH / "Patterns/Uright") {}
};

//********************************
// Single-counter dataset generator
//********************************

class SingleCounterDatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<always_realizable> {
public:
  SingleCounterDatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(
            DATASETS_PATH / "Two-player-Game/Single-Counter/System-first") {}
};

//********************************
// Double-counter dataset generator
//********************************

class DoubleCounterDatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<always_realizable> {
public:
  DoubleCounterDatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(
            DATASETS_PATH / "Two-player-Game/Double-Counter/System-first") {}
};

//********************************
// Nim dataset generator
//********************************

struct is_nim1_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    // only first is unrealizable
    if (index == 0)
      return SynthesisResult::UNREALIZABLE;
    // the other instances are realizable
    return SynthesisResult::REALIZABLE;
  }
};
struct is_nim2_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    // only first is realizable
    if (index == 0)
      return SynthesisResult::REALIZABLE;
    // the other instances are unrealizable
    return SynthesisResult::UNREALIZABLE;
  }
};

class Nim1DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_nim1_problem_realizable> {
public:
  Nim1DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(
            DATASETS_PATH / "Two-player-Game/Nim/nim_01/System-first") {}
};
class Nim2DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_nim2_problem_realizable> {
public:
  Nim2DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(
            DATASETS_PATH / "Two-player-Game/Nim/nim_02/System-first") {}
};
class Nim3DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<always_unrealizable> {
public:
  Nim3DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(
            DATASETS_PATH / "Two-player-Game/Nim/nim_03/System-first") {}
};
class Nim4DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<always_unrealizable> {
public:
  Nim4DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(
            DATASETS_PATH / "Two-player-Game/Nim/nim_04/System-first") {}
};
class Nim5DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<always_unrealizable> {
public:
  Nim5DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(
            DATASETS_PATH / "Two-player-Game/Nim/nim_05/System-first") {}
};

//********************************
// Lydia Random dataset generator, case 03-50
//********************************

static const std::map<size_t, bool> case_03_50_realizability = {
    {1, false},  {2, false},  {3, false},  {4, false},  {5, true},
    {6, false},  {7, true},   {8, false},  {9, false},  {10, false},
    {11, true},  {12, false}, {13, false}, {14, true},  {15, true},
    {16, true},  {17, false}, {18, false}, {19, false}, {20, true},
    {21, true},  {22, false}, {23, true},  {24, false}, {25, false},
    {26, false}, {27, true},  {28, false}, {29, true},  {30, true},
    {31, false}, {32, false}, {33, false}, {34, false}, {35, true},
    {36, true},  {37, false}, {38, false}, {39, true},  {40, false},
    {41, false}, {42, true},  {43, true},  {44, false}, {45, false},
    {46, true},  {47, false}, {48, true},  {49, true},  {50, true},
};

static const std::set<size_t> case_03_50_tractable_instances = {
    1,  4,  5,  7,  8,  9,  11, 12, 14, 15, 16, 18, 19, 20, 21, 23, 24,
    26, 27, 28, 31, 33, 34, 36, 39, 42, 43, 44, 45, 46, 47, 48, 49, 50};

struct is_case_03_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_03_50_realizability.find(index + 1) ==
        case_03_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_03_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_03_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_03_50_problem_realizable> {
public:
  LydiaRandom_03_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_03_50") {}
};

bool tractable_lydia_random_03_50(const problem_t &problem) {
  return check_index_in_set(problem, case_03_50_tractable_instances);
}

//********************************
// Lydia Random dataset generator, case 04-50
//********************************

static const std::map<size_t, bool> case_04_50_realizability = {
    {1, true},   {2, true},   {3, false},  {4, false},  {5, false},
    {6, true},   {7, false},  {8, true},   {9, true},   {10, false},
    {11, false}, {12, false}, {13, false}, {14, true},  {15, true},
    {16, false}, {17, false}, {18, true},  {19, false}, {20, false},
    {21, false}, {22, false}, {23, false}, {24, false}, {25, true},
    {26, false}, {27, false}, {28, false}, {29, false}, {30, true},
    {31, true},  {32, false}, {33, false}, {34, false}, {35, false},
    {36, true},  {37, true},  {38, true},  {39, false}, {40, false},
    {41, false}, {42, false}, {43, false}, {44, false}, {45, true},
    {46, false}, {47, false}, {48, false}, {49, true},  {50, false},
};

static const std::set<size_t> case_04_50_tractable_instances = {
    1, 2, 3, 8, 9, 12, 13, 18, 27, 28, 31, 33, 34, 36, 38, 45, 46, 50};

struct is_case_04_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_04_50_realizability.find(index + 1) ==
        case_04_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_04_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_04_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_04_50_problem_realizable> {
public:
  LydiaRandom_04_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_04_50") {}
};

bool tractable_lydia_random_04_50(const problem_t &problem) {
  return check_index_in_set(problem, case_04_50_tractable_instances);
}

//********************************
// Lydia Random dataset generator, case 05-50
//********************************

static const std::map<size_t, bool> case_05_50_realizability = {
    {1, false},  {2, false},  {3, true},   {4, false},  {5, false},
    {6, false},  {7, true},   {8, false},  {9, false},  {10, false},
    {11, false}, {12, false}, {13, true},  {14, true},  {15, false},
    {16, true},  {17, false}, {18, false}, {19, true},  {20, false},
    {21, true},  {22, false}, {23, false}, {24, false}, {25, true},
    {26, false}, {27, true},  {28, false}, {29, true},  {30, true},
    {31, false}, {32, false}, {33, false}, {34, false}, {35, false},
    {36, true},  {37, false}, {38, true},  {39, false}, {40, true},
    {41, true},  {42, false}, {43, false}, {44, true},  {45, true},
    {46, false}, {47, false}, {48, true},  {49, false}, {50, true},
};
static const std::set<size_t> case_05_50_tractable_instances = {
    7, 9, 13, 15, 16, 17, 20, 21, 25, 27, 30, 31, 33, 35, 36, 40, 41, 43, 44};

struct is_case_05_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_05_50_realizability.find(index + 1) ==
        case_05_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_05_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_05_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_05_50_problem_realizable> {
public:
  LydiaRandom_05_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_05_50") {}
};

bool tractable_lydia_random_05_50(const problem_t &problem) {
  return check_index_in_set(problem, case_05_50_tractable_instances);
}
//********************************
// Lydia Random dataset generator, case 06-50
//********************************

static const std::map<size_t, bool> case_06_50_realizability = {
    {1, true},   {2, false},  {3, true},   {4, true},   {5, false},
    {6, false},  {7, false},  {8, true},   {9, false},  {10, true},
    {11, false}, {12, true},  {13, true},  {14, false}, {15, false},
    {16, false}, {17, true},  {18, false}, {19, false}, {20, true},
    {21, false}, {22, false}, {23, false}, {24, false}, {25, true},
    {26, true},  {27, false}, {28, true},  {29, false}, {30, false},
    {31, true},  {32, true},  {33, false}, {34, false}, {35, false},
    {36, false}, {37, false}, {38, false}, {39, false}, {40, true},
    {41, false}, {42, false}, {43, true},  {44, false}, {45, true},
    {46, false}, {47, false}, {48, true},  {49, true},  {50, true},
};

static const std::set<size_t> case_06_50_tractable_instances = {
    1, 2, 4, 9, 12, 13, 17, 22, 23, 29, 35};

struct is_case_06_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_06_50_realizability.find(index + 1) ==
        case_06_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_06_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_06_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_06_50_problem_realizable> {
public:
  LydiaRandom_06_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_06_50") {}
};

bool tractable_lydia_random_06_50(const problem_t &problem) {
  return check_index_in_set(problem, case_06_50_tractable_instances);
}

//********************************
// Lydia Random dataset generator, case 07-50
//********************************

static const std::map<size_t, bool> case_07_50_realizability = {
    {1, false},  {2, true},   {4, true},   {5, false},  {6, true},
    {9, false},  {10, true},  {12, false}, {13, true},  {15, true},
    {16, true},  {17, false}, {18, false}, {20, false}, {21, true},
    {22, false}, {24, false}, {25, false}, {26, false}, {27, false},
    {28, false}, {29, true},  {30, false}, {32, true},  {33, true},
    {34, false}, {35, true},  {36, false}, {38, false}, {39, true},
    {40, true},  {41, true},  {45, true},  {46, false}, {47, false},
    {48, false}, {49, false},
};

static const std::set<size_t> case_07_50_tractable_instances = {
    4, 6, 13, 15, 18, 29, 32, 33, 34, 35, 38, 41};

struct is_case_07_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_07_50_realizability.find(index + 1) ==
        case_07_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_07_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_07_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_07_50_problem_realizable> {
public:
  LydiaRandom_07_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_07_50") {}
};

bool tractable_lydia_random_07_50(const problem_t &problem) {
  return check_index_in_set(problem, case_07_50_tractable_instances);
}

//********************************
// Lydia Random dataset generator, case 08-50
//********************************

static const std::map<size_t, bool> case_08_50_realizability = {
    {1, false},  {3, true},   {4, true},  {5, false},  {6, false},  {7, false},
    {8, true},   {10, false}, {11, true}, {12, false}, {14, false}, {15, false},
    {16, true},  {18, true},  {20, true}, {21, true},  {22, false}, {23, false},
    {24, true},  {25, false}, {26, true}, {28, true},  {30, true},  {31, true},
    {32, true},  {33, true},  {34, true}, {35, false}, {36, true},  {37, true},
    {38, false}, {39, false}, {40, true}, {41, true},  {42, true},  {43, false},
    {44, false}, {45, false}, {46, true}, {47, true},  {49, true},
};

static const std::set<size_t> case_08_50_tractable_instances = {4,  11, 30, 31,
                                                                36, 37, 47};

struct is_case_08_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_08_50_realizability.find(index + 1) ==
        case_08_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_08_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_08_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_08_50_problem_realizable> {
public:
  LydiaRandom_08_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_08_50") {}
};

bool tractable_lydia_random_08_50(const problem_t &problem) {
  return check_index_in_set(problem, case_08_50_tractable_instances);
}

//********************************
// Lydia Random dataset generator, case 09-50
//********************************

static const std::map<size_t, bool> case_09_50_realizability = {
    {1, true},   {4, false},  {8, false}, {9, true},   {12, true},  {13, false},
    {15, false}, {17, false}, {19, true}, {20, false}, {21, false}, {22, true},
    {23, false}, {24, true},  {25, true}, {27, false}, {29, true},  {32, false},
    {34, true},  {35, true},  {42, true}, {45, true},  {48, true},  {50, true},
};

static const std::set<size_t> case_09_50_tractable_instances = {12, 25};

struct is_case_09_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_09_50_realizability.find(index + 1) ==
        case_09_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_09_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_09_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_09_50_problem_realizable> {
public:
  LydiaRandom_09_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_09_50") {}
};

bool tractable_lydia_random_09_50(const problem_t &problem) {
  return check_index_in_set(problem, case_09_50_tractable_instances);
}

//********************************
// Lydia Random dataset generator, case 10-50
//********************************

static const std::map<size_t, bool> case_10_50_realizability = {
    {1, false},  {3, true},   {4, true},  {6, true},   {7, true},   {8, false},
    {10, false}, {12, true},  {13, true}, {14, true},  {17, true},  {21, true},
    {25, true},  {27, false}, {28, true}, {29, false}, {30, false}, {33, true},
    {34, false}, {36, true},  {39, true}, {41, false}, {42, false}, {43, true},
    {44, false}, {45, false}, {50, true},
};

static const std::set<size_t> case_10_50_tractable_instances = {
    3, 6, 14, 25, 27, 33, 36, 39, 42, 43};

struct is_case_10_50_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (case_10_50_realizability.find(index + 1) ==
        case_10_50_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return case_10_50_realizability.at(index + 1)
               ? SynthesisResult::REALIZABLE
               : SynthesisResult::UNREALIZABLE;
  }
};
class LydiaRandom_10_50_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_case_10_50_problem_realizable> {
public:
  LydiaRandom_10_50_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Lydia/case_10_50") {}
};

bool tractable_lydia_random_10_50(const problem_t &problem) {
  return check_index_in_set(problem, case_10_50_tractable_instances);
}

//********************************
// Syft Random dataset generator, set 1
//********************************

static const std::map<size_t, bool> syft_1_realizability = {
    {1, true},    {2, true},    {3, false},   {4, true},    {5, false},
    {6, true},    {7, true},    {8, true},    {9, true},    {10, true},
    {11, false},  {12, true},   {13, true},   {14, false},  {15, false},
    {16, false},  {17, false},  {18, false},  {19, true},   {20, false},
    {21, true},   {22, true},   {23, false},  {24, false},  {25, false},
    {26, true},   {27, false},  {28, false},  {29, true},   {30, true},
    {31, false},  {32, true},   {33, true},   {34, false},  {35, true},
    {36, false},  {37, true},   {38, true},   {39, false},  {40, true},
    {41, false},  {42, true},   {43, false},  {44, true},   {45, true},
    {46, true},   {47, true},   {48, true},   {49, false},  {50, true},
    {51, true},   {52, true},   {53, true},   {54, false},  {55, false},
    {56, false},  {57, false},  {58, false},  {59, true},   {60, true},
    {61, true},   {62, false},  {63, false},  {64, true},   {65, false},
    {66, true},   {67, true},   {68, false},  {69, true},   {70, true},
    {71, true},   {72, false},  {73, false},  {74, true},   {75, false},
    {76, true},   {77, false},  {78, false},  {79, true},   {80, false},
    {81, true},   {82, false},  {83, true},   {84, true},   {85, true},
    {86, true},   {87, false},  {88, true},   {89, false},  {90, false},
    {91, false},  {92, false},  {93, true},   {94, true},   {95, true},
    {96, false},  {97, false},  {98, true},   {99, true},   {100, false},
    {101, true},  {102, true},  {103, false}, {104, false}, {105, true},
    {106, false}, {107, false}, {108, true},  {109, true},  {110, true},
    {111, true},  {112, false}, {113, false}, {114, true},  {115, true},
    {116, true},  {117, false}, {118, true},  {119, true},  {120, false},
    {121, false}, {122, false}, {123, false}, {124, false}, {125, false},
    {126, true},  {127, false}, {128, true},  {129, false}, {130, true},
    {131, true},  {132, true},  {133, false}, {134, true},  {135, true},
    {136, false}, {137, false}, {138, true},  {139, false}, {140, false},
    {141, false}, {142, true},  {143, false}, {144, true},  {145, true},
    {146, false}, {147, true},  {148, false}, {149, true},  {150, false},
    {151, false}, {152, false}, {153, false}, {154, false}, {155, false},
    {156, true},  {157, false}, {158, false}, {159, true},  {160, true},
    {161, false}, {162, true},  {163, true},  {164, false}, {165, false},
    {166, false}, {167, false}, {168, true},  {169, true},  {170, false},
    {171, false}, {172, false}, {173, false}, {174, true},  {175, false},
    {176, false}, {177, false}, {178, false}, {179, true},  {180, false},
    {181, false}, {182, true},  {183, true},  {184, true},  {185, true},
    {186, true},  {187, true},  {188, true},  {189, true},  {190, false},
    {191, true},  {192, false}, {193, true},  {194, false}, {195, false},
    {196, true},  {197, true},  {198, true},  {199, true},  {200, true},
};

static const std::set<size_t> syft_1_tractable_instances = {
    1,   2,   3,   4,   6,   7,   8,   9,   10,  12,  13,  19,  21,  22,  26,
    29,  30,  32,  33,  35,  37,  38,  40,  42,  44,  45,  46,  47,  48,  50,
    51,  52,  53,  59,  60,  61,  63,  64,  66,  67,  69,  70,  71,  74,  76,
    77,  79,  80,  81,  83,  84,  85,  86,  88,  92,  93,  94,  95,  98,  99,
    100, 101, 102, 105, 108, 109, 110, 111, 114, 115, 116, 118, 119, 123, 126,
    128, 130, 131, 132, 133, 134, 135, 138, 142, 144, 145, 147, 149, 153, 155,
    156, 159, 160, 162, 163, 167, 168, 169, 174, 175, 178, 179, 182, 183, 184,
    185, 186, 187, 188, 189, 190, 191, 193, 196, 197, 198, 199, 200};

struct is_syft_1_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (syft_1_realizability.find(index + 1) == syft_1_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return syft_1_realizability.at(index + 1) ? SynthesisResult::REALIZABLE
                                              : SynthesisResult::UNREALIZABLE;
  }
};
class SyftRandom_1_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_syft_1_problem_realizable> {
public:
  SyftRandom_1_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Syft/syft_1") {}
};

bool tractable_syft_random_1(const problem_t &problem) {
  return check_index_in_set(problem, syft_1_tractable_instances);
}

//********************************
// Syft Random dataset generator, set 2
//********************************

static const std::map<size_t, bool> syft_2_realizability = {
    {1, false},   {2, true},    {3, false},   {4, false},   {5, false},
    {6, false},   {7, true},    {8, false},   {9, false},   {10, false},
    {11, false},  {12, false},  {13, false},  {14, false},  {15, false},
    {16, false},  {17, false},  {18, false},  {19, false},  {20, false},
    {21, false},  {22, true},   {23, true},   {24, false},  {25, false},
    {26, false},  {27, true},   {28, false},  {29, false},  {30, false},
    {31, false},  {32, true},   {33, true},   {34, false},  {35, true},
    {36, false},  {37, false},  {38, false},  {39, true},   {40, true},
    {41, false},  {42, false},  {43, false},  {44, false},  {45, false},
    {46, false},  {47, false},  {48, false},  {49, false},  {50, false},
    {51, false},  {52, false},  {53, true},   {54, true},   {55, true},
    {56, false},  {57, false},  {58, false},  {59, false},  {60, false},
    {61, true},   {62, true},   {63, false},  {64, false},  {65, false},
    {66, false},  {67, false},  {68, false},  {69, true},   {70, false},
    {71, false},  {72, false},  {73, false},  {74, true},   {75, false},
    {76, false},  {77, false},  {78, false},  {79, true},   {80, true},
    {81, false},  {82, false},  {83, true},   {84, false},  {85, false},
    {86, true},   {87, false},  {88, false},  {89, true},   {90, false},
    {91, false},  {92, false},  {93, false},  {94, false},  {95, false},
    {96, true},   {97, false},  {98, true},   {99, true},   {100, false},
    {101, false}, {102, false}, {103, false}, {104, true},  {105, false},
    {106, false}, {107, false}, {108, false}, {109, false}, {110, false},
    {111, true},  {112, false}, {113, false}, {114, false}, {115, false},
    {116, false}, {117, true},  {118, false}, {119, false}, {120, false},
    {121, false}, {122, true},  {123, false}, {124, false}, {125, false},
    {126, false}, {127, false}, {128, false}, {129, true},  {130, false},
    {131, true},  {132, false}, {133, true},  {134, false}, {135, true},
    {136, true},  {137, true},  {138, true},  {139, false}, {140, true},
    {141, true},  {142, false}, {143, true},  {144, false}, {145, false},
    {146, false}, {147, false}, {148, true},  {149, true},  {150, false},
    {151, false}, {152, true},  {153, true},  {154, true},  {155, false},
    {156, false}, {157, false}, {158, false}, {159, true},  {160, false},
    {161, false}, {162, false}, {163, true},  {164, false}, {165, true},
    {166, false}, {167, false}, {168, false}, {169, false}, {170, false},
    {171, true},  {172, false}, {173, true},  {174, false}, {175, true},
    {176, false}, {177, false}, {178, false}, {179, false}, {180, true},
    {181, false}, {182, false}, {183, false}, {184, true},  {185, false},
    {186, true},  {187, false}, {188, false}, {189, false}, {190, false},
    {191, false}, {192, false}, {193, false}, {194, false}, {195, false},
    {196, false}, {197, true},  {198, false}, {199, false}, {200, false},
};

static const std::set<size_t> syft_2_tractable_instances = {
    2,   7,   14,  17,  18,  22,  23,  24,  27,  32,  33,  35,  39,
    40,  41,  50,  54,  55,  61,  62,  64,  68,  69,  72,  74,  77,
    79,  80,  83,  86,  87,  88,  89,  93,  98,  99,  104, 106, 111,
    117, 122, 129, 131, 132, 133, 135, 136, 137, 138, 139, 140, 141,
    143, 147, 148, 149, 152, 153, 154, 157, 159, 160, 162, 165, 168,
    171, 173, 175, 180, 181, 184, 186, 190, 191, 194, 197, 199};

struct is_syft_2_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (syft_2_realizability.find(index + 1) == syft_2_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return syft_2_realizability.at(index + 1) ? SynthesisResult::REALIZABLE
                                              : SynthesisResult::UNREALIZABLE;
  }
};
class SyftRandom_2_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_syft_2_problem_realizable> {
public:
  SyftRandom_2_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Syft/syft_2") {}
};

bool tractable_syft_random_2(const problem_t &problem) {
  return check_index_in_set(problem, syft_2_tractable_instances);
}

//********************************
// Syft Random dataset generator, set 3
//********************************

static const std::map<size_t, bool> syft_3_realizability = {
    {1, false},   {2, false},   {3, true},    {4, false},   {5, true},
    {6, false},   {7, false},   {8, false},   {9, false},   {10, false},
    {11, false},  {12, false},  {13, true},   {14, false},  {15, false},
    {16, false},  {17, false},  {18, false},  {19, false},  {20, true},
    {21, false},  {22, false},  {23, false},  {24, false},  {25, true},
    {26, false},  {27, true},   {28, false},  {29, false},  {30, false},
    {31, false},  {32, true},   {33, false},  {34, false},  {35, false},
    {36, true},   {37, false},  {38, false},  {39, false},  {40, false},
    {41, false},  {42, false},  {43, false},  {44, false},  {45, true},
    {46, false},  {47, false},  {48, false},  {49, false},  {50, false},
    {51, false},  {52, false},  {53, false},  {54, false},  {55, true},
    {56, false},  {57, false},  {58, false},  {59, false},  {60, false},
    {61, false},  {62, false},  {63, false},  {64, true},   {65, false},
    {66, true},   {67, true},   {68, true},   {69, true},   {70, false},
    {71, false},  {72, false},  {73, false},  {74, false},  {75, false},
    {76, false},  {77, false},  {78, false},  {79, true},   {80, true},
    {81, false},  {82, false},  {83, false},  {84, false},  {85, false},
    {86, false},  {87, false},  {88, false},  {89, true},   {90, false},
    {91, false},  {92, false},  {93, false},  {94, false},  {95, false},
    {96, false},  {97, false},  {98, false},  {99, false},  {100, false},
    {101, false}, {102, false}, {103, false}, {104, false}, {105, false},
    {106, false}, {107, true},  {108, false}, {109, false}, {110, false},
    {111, false}, {112, false}, {113, false}, {114, false}, {115, false},
    {116, false}, {117, false}, {118, false}, {119, false}, {120, false},
    {121, false}, {122, false}, {123, true},  {124, false}, {125, false},
    {126, false}, {127, false}, {128, false}, {129, false}, {130, false},
    {131, false}, {132, false}, {133, false}, {134, false}, {135, false},
    {136, false}, {137, false}, {138, false}, {139, false}, {140, false},
    {141, false}, {142, false}, {143, false}, {144, false}, {145, false},
    {146, false}, {147, false}, {148, false}, {149, false}, {150, true},
    {151, false}, {152, true},  {153, false}, {154, false}, {155, false},
    {156, false}, {157, false}, {158, true},  {159, false}, {160, false},
    {161, true},  {162, false}, {163, false}, {164, false}, {165, false},
    {166, false}, {167, true},  {168, false}, {169, false}, {170, false},
    {171, false}, {172, false}, {173, false}, {174, false}, {175, false},
    {176, true},  {177, false}, {178, false}, {179, false}, {180, true},
    {181, false}, {182, false}, {183, false}, {184, false}, {185, false},
    {186, false}, {187, false}, {188, false}, {189, false}, {190, true},
    {191, false}, {192, true},  {193, false}, {194, false}, {195, false},
    {196, false}, {197, false}, {198, false}, {199, false}, {200, false},
};

static const std::set<size_t> syft_3_tractable_instances = {
    3,   5,   11,  13,  15,  26,  32,  36,  42,  52,  55,  64,  66,  68,
    80,  81,  87,  89,  91,  94,  95,  99,  102, 106, 107, 113, 115, 118,
    123, 143, 145, 152, 158, 159, 167, 176, 180, 183, 190, 192};

struct is_syft_3_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (syft_3_realizability.find(index + 1) == syft_3_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return syft_3_realizability.at(index + 1) ? SynthesisResult::REALIZABLE
                                              : SynthesisResult::UNREALIZABLE;
  }
};
class SyftRandom_3_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_syft_3_problem_realizable> {
public:
  SyftRandom_3_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Syft/syft_3") {}
};

bool tractable_syft_random_3(const problem_t &problem) {
  return check_index_in_set(problem, syft_3_tractable_instances);
}

//********************************
// Syft Random dataset generator, set 4
//********************************

static const std::map<size_t, bool> syft_4_realizability = {
    {1, false},   {2, false},   {3, true},    {4, false},   {5, false},
    {6, false},   {7, false},   {8, false},   {9, false},   {10, false},
    {11, false},  {12, false},  {13, false},  {14, false},  {15, false},
    {16, false},  {17, false},  {18, false},  {19, false},  {20, false},
    {21, false},  {22, false},  {23, false},  {24, false},  {25, false},
    {26, false},  {27, false},  {28, false},  {29, false},  {30, false},
    {31, false},  {32, false},  {33, false},  {34, false},  {35, false},
    {36, false},  {37, false},  {38, false},  {39, false},  {40, false},
    {41, false},  {42, false},  {43, false},  {44, false},  {45, false},
    {46, false},  {47, false},  {48, false},  {49, false},  {50, false},
    {51, false},  {52, false},  {53, false},  {54, false},  {55, false},
    {56, false},  {57, false},  {58, false},  {59, false},  {60, false},
    {61, false},  {62, false},  {63, false},  {64, false},  {65, false},
    {66, false},  {67, false},  {68, false},  {69, false},  {70, false},
    {71, true},   {72, false},  {73, false},  {74, false},  {75, false},
    {76, false},  {77, false},  {78, false},  {79, false},  {80, false},
    {81, false},  {82, false},  {83, false},  {84, false},  {85, false},
    {86, false},  {87, false},  {88, false},  {89, false},  {90, false},
    {91, false},  {92, false},  {93, false},  {94, false},  {95, false},
    {96, false},  {97, false},  {98, true},   {99, false},  {100, false},
    {101, false}, {102, false}, {103, false}, {104, false}, {105, false},
    {106, false}, {107, true},  {108, false}, {109, false}, {110, false},
    {111, false}, {112, false}, {113, false}, {114, false}, {115, false},
    {116, true},  {117, false}, {118, false}, {119, false}, {120, true},
    {121, false}, {122, false}, {123, false}, {125, false}, {126, false},
    {127, false}, {128, false}, {129, false}, {130, false}, {131, false},
    {132, false}, {133, false}, {134, false}, {135, false}, {136, false},
    {137, true},  {138, false}, {139, false}, {140, false}, {141, false},
    {142, false}, {143, false}, {144, false}, {145, true},  {146, false},
    {147, false}, {148, false}, {149, false}, {150, false}, {151, false},
    {152, false}, {153, false}, {154, false}, {155, false}, {156, false},
    {157, false}, {158, false}, {159, false}, {160, false}, {161, false},
    {162, false}, {163, true},  {164, false}, {165, false}, {166, true},
    {167, false}, {168, false}, {169, false}, {170, false}, {171, false},
    {172, false}, {173, false}, {174, false}, {175, false}, {176, true},
    {177, false}, {178, false}, {179, false}, {180, false}, {181, false},
    {182, false}, {183, false}, {184, false}, {185, false}, {186, false},
    {187, false}, {188, false}, {189, false}, {190, false}, {191, true},
    {192, true},  {193, false}, {194, false}, {195, false}, {196, false},
    {197, false}, {198, false}, {199, false}, {200, false},
};

static const std::set<size_t> syft_4_tractable_instances = {3,  71, 81,
                                                            86, 98, 176};

struct is_syft_4_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (syft_4_realizability.find(index + 1) == syft_4_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return syft_4_realizability.at(index + 1) ? SynthesisResult::REALIZABLE
                                              : SynthesisResult::UNREALIZABLE;
  }
};
class SyftRandom_4_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_syft_4_problem_realizable> {
public:
  SyftRandom_4_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Syft/syft_4") {}
};

bool tractable_syft_random_4(const problem_t &problem) {
  return check_index_in_set(problem, syft_4_tractable_instances);
}
//********************************
// Syft Random dataset generator, set 5
//********************************

static const std::map<size_t, bool> syft_5_realizability = {
    {1, false},   {2, false},   {3, false},   {4, false},   {5, false},
    {6, false},   {7, false},   {8, false},   {9, false},   {10, false},
    {11, false},  {12, false},  {13, false},  {14, false},  {15, false},
    {16, false},  {17, false},  {18, false},  {19, false},  {20, false},
    {21, false},  {22, false},  {23, false},  {24, false},  {25, false},
    {26, false},  {27, false},  {28, false},  {29, false},  {30, false},
    {31, false},  {32, false},  {33, false},  {34, false},  {35, false},
    {36, false},  {37, false},  {38, false},  {39, false},  {40, false},
    {41, false},  {42, false},  {43, false},  {44, false},  {45, false},
    {46, false},  {47, false},  {48, false},  {49, false},  {50, false},
    {51, false},  {52, false},  {53, false},  {54, false},  {55, false},
    {56, false},  {57, true},   {58, false},  {59, false},  {60, false},
    {61, false},  {62, false},  {63, false},  {64, false},  {65, true},
    {66, false},  {67, false},  {68, true},   {69, false},  {70, false},
    {71, false},  {72, false},  {73, false},  {74, false},  {75, false},
    {76, false},  {78, false},  {79, false},  {80, false},  {81, false},
    {82, false},  {83, false},  {84, false},  {85, false},  {87, false},
    {88, false},  {89, false},  {90, false},  {91, false},  {92, false},
    {93, false},  {94, false},  {95, false},  {96, true},   {97, false},
    {98, false},  {99, true},   {100, false}, {101, false}, {102, false},
    {103, false}, {104, false}, {105, false}, {106, false}, {107, false},
    {108, false}, {109, false}, {110, false}, {111, false}, {112, false},
    {113, false}, {114, false}, {115, false}, {116, false}, {117, false},
    {118, false}, {119, false}, {120, false}, {121, true},  {122, false},
    {123, false}, {124, false}, {125, false}, {126, false}, {127, false},
    {128, false}, {129, false}, {130, false}, {131, false}, {132, false},
    {133, false}, {135, false}, {136, false}, {137, false}, {138, false},
    {139, false}, {140, false}, {141, false}, {142, false}, {143, false},
    {144, false}, {145, false}, {146, false}, {147, false}, {148, false},
    {149, false}, {150, false}, {151, false}, {152, false}, {153, true},
    {154, false}, {155, false}, {156, false}, {157, false}, {158, false},
    {159, false}, {160, false}, {161, false}, {162, false}, {163, false},
    {164, false}, {165, false}, {166, false}, {167, false}, {168, false},
    {169, false}, {170, false}, {171, false}, {172, false}, {173, false},
    {174, false}, {175, false}, {176, false}, {177, false}, {178, false},
    {179, false}, {180, false}, {181, false}, {182, false}, {183, false},
    {184, false}, {185, false}, {186, false}, {187, false}, {188, true},
    {189, false}, {190, false}, {191, false}, {192, false}, {193, false},
    {194, false}, {195, false}, {196, true},  {197, false}, {198, false},
    {199, false}, {200, false},
};

static const std::set<size_t> syft_5_tractable_instances = {38,  71,  96, 125,
                                                            153, 162, 187};

struct is_syft_5_problem_realizable {
  SynthesisResult operator()(size_t index, const std::filesystem::path &formula) {
    if (syft_5_realizability.find(index + 1) == syft_5_realizability.end()) {
      return SynthesisResult::UNKNOWN;
    }
    return syft_5_realizability.at(index + 1) ? SynthesisResult::REALIZABLE
                                              : SynthesisResult::UNREALIZABLE;
  }
};
class SyftRandom_5_DatasetProblemGenerator
    : public AbstractDatasetProblemGenerator<is_syft_5_problem_realizable> {
public:
  SyftRandom_5_DatasetProblemGenerator()
      : AbstractDatasetProblemGenerator(DATASETS_PATH /
                                        "Random/Syft/syft_5") {}
};

bool tractable_syft_random_5(const problem_t &problem) {
  return check_index_in_set(problem, syft_5_tractable_instances);
}

}
