#include <filesystem>
#include <memory>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/SymbolicStateDfa.h"
#include "Utils.h"
#include "Preprocessing.h"


int main(int argc, char ** argv) {

    // parse TLSF file
    std::filesystem::path ROOT_DIRECTORY = __ROOT_DIRECTORY;
    std::filesystem::path finding_nemo_dir = ROOT_DIRECTORY / "examples" / "TLSF" / "GR1benchmarks" / "finding_nemo";
    std::filesystem::path finding_nemo_1 = finding_nemo_dir / "finding_nemo_1.tlsf";
    std::filesystem::path finding_nemo_1_env_gr1 = finding_nemo_dir / "finding_nemo_1_env_gr1.txt";
    std::filesystem::path finding_nemo_1_env_safety = finding_nemo_dir / "finding_nemo_1_env_safety.ltlf";
    std::filesystem::path finding_nemo_1_agent_safety = finding_nemo_dir / "finding_nemo_1_agn_safety.ltlf";
    std::filesystem::path path_to_slugs_dir = ROOT_DIRECTORY / "submodules" / "slugs";

    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    Syft::TLSFArgs args = Syft::parse_tlsf(driver, finding_nemo_1.string());
    auto var_mgr = Syft::build_var_mgr(args.partition);
    whitemech::lydia::ltlf_ptr agent_safety_formula = Syft::parse_formula(
        driver,
        Syft::read_assumption_file_if_file_specified(finding_nemo_1_agent_safety)
    );
    whitemech::lydia::ltlf_ptr env_safety_formula = Syft::parse_formula(
        driver,
        Syft::read_assumption_file_if_file_specified(finding_nemo_1_env_safety)
    );

    std::cout << "Building the DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);
    std::cout << "Building the DFA of the agent safety formula..." << std::endl;
    Syft::SymbolicStateDfa dfa_agn = Syft::do_dfa_construction(*agent_safety_formula, var_mgr);
    std::cout << "Building the DFA of the environment safety formula..." << std::endl;
    Syft::SymbolicStateDfa dfa_env = Syft::do_dfa_construction(*env_safety_formula, var_mgr);

    std::cout << "Solving the DFA game with GR(1) env assumptions..." << std::endl;
    Syft::GR1 gr1 = Syft::GR1::read_from_gr1_file(var_mgr, finding_nemo_1_env_gr1.string());
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
    Syft::GR1LTLfSynthesizer synthesizer(var_mgr, gr1, dfa_env,
                                         dfa, dfa_agn, path_to_slugs_dir.string(), "problem");
    Syft::SynthesisResult result = synthesizer.run();

    if (result.realizability) {
        std::cout << "Specification is realizable!" << std::endl;
    }
    else {
        std::cout << "Specification is unrealizable!" << std::endl;
    }

}