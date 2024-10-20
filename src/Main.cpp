#include <memory>

#include "cli/base.hpp"
#include "cli/fairness.hpp"
#include "cli/gr1.hpp"
#include "cli/stability.hpp"
#include "cli/synthesis.hpp"
#include "cli/maxset.hpp"
#include "game/InputOutputPartition.h"
#include "Preprocessing.h"
#include "Utils.h"
#include <lydia/parser/ltlf/driver.hpp>
#include <CLI/CLI.hpp>


int main(int argc, char ** argv) {

    CLI::App app {
        "LydiaSyft: A compositional synthesizer for Linear Temporal Logic on finite traces (LTLf)"
    };
    app.set_help_all_flag("--help-all", "Expand all help");

    // shared arguments
    bool print_strategy = false;
    app.add_flag("-p, --print-strategy", print_strategy, "Print out the synthesized strategy (default: false)");

    bool print_times = false;
    app.add_flag("-t, --print-times", print_times, "Print out running times of each step (default: false)");

    std::string formula_file;
    std::optional<std::string> path_to_syfco_opt = std::nullopt;
    std::string assumption_file;
    std::string gr1_file;
    std::optional<std::string> env_safety_file;
    std::optional<std::string> agent_safety_file;
    std::string path_to_slugs;

    // 'synthesis' subcommand
    CLI::App *synthesis = app.add_subcommand("synthesis", "solve a classical LTLf synthesis problem");
    Syft::add_spec_file_option(synthesis, formula_file);
    Syft::add_syfco_option(synthesis, path_to_syfco_opt);

    // 'maxset' subcommand
    CLI::App *maxset = app.add_subcommand("maxset", "solve LTLf synthesis with maximally permissive strategies");
    Syft::add_spec_file_option(maxset, formula_file);
    Syft::add_syfco_option(maxset, path_to_syfco_opt);

    // 'fairness' subcommand
    CLI::App *fairness = app.add_subcommand("fairness", "solve LTLf synthesis with fairness assumptions");
    Syft::add_spec_file_option(fairness, formula_file);
    Syft::add_syfco_option(fairness, path_to_syfco_opt);
    Syft::add_assumption_file_option(fairness, assumption_file);

    // 'stability' subcommand
    CLI::App *stability = app.add_subcommand("stability", "solve LTLf synthesis with stability assumptions");
    Syft::add_spec_file_option(stability, formula_file);
    Syft::add_syfco_option(stability, path_to_syfco_opt);
    Syft::add_assumption_file_option(stability, assumption_file);

    // 'gr1' subcommand
    CLI::App *gr1 = app.add_subcommand("gr1", "Solve LTLf synthesis with GR(1) conditions");
    Syft::add_spec_file_option(gr1, formula_file);
    Syft::add_syfco_option(gr1, path_to_syfco_opt);
    Syft::add_slugs_option(gr1, path_to_slugs);
    Syft::add_gr1_file_option(gr1, gr1_file);
    Syft::add_env_safety_file_option(gr1, env_safety_file);
    Syft::add_agent_safety_file_option(gr1, agent_safety_file);

    app.require_subcommand(1);

    // the CLI11 APIs add_option sets the value to empty string ""; resetting it to null value
    path_to_syfco_opt = std::nullopt;

    CLI11_PARSE(app, argc, argv);

    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();

    std::string path_to_syfco = Syft::find_syfco_path(path_to_syfco_opt);

    if (app.got_subcommand(synthesis)){
        Syft::SynthesisRunner(driver, formula_file, path_to_syfco, print_strategy, print_times).run();
    }
    else if (app.got_subcommand(maxset)){
        Syft::MaxSetRunner(driver, formula_file, path_to_syfco, print_strategy, print_times).run();
    }
    else if (app.got_subcommand(fairness)) {
        Syft::FairnessRunner(driver, formula_file, path_to_syfco, assumption_file, print_strategy, print_times).run();
    }
    else if (app.got_subcommand(stability)) {
        Syft::StabilityRunner(driver, formula_file, path_to_syfco, assumption_file, print_times, print_times).run();
    }
    else if (app.got_subcommand(gr1)) {
        Syft::GR1Runner(driver, formula_file, path_to_syfco, path_to_slugs, gr1_file, env_safety_file, agent_safety_file, print_strategy, print_times).run();
    }
    else {
        assert(false && "CLI Parsing Error");
    }

    return 0;
}

