//
// Created by shuzhu on 19/05/23.
//

#include "Parser.h"

#include <fstream>
#include <filesystem>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>

namespace Syft {

    Parser::Parser()
    {}

    std::string Parser::ltrim(const std::string &s) {
        const std::string WHITESPACE = " \n\r\t\f\v";
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == std::string::npos) ? "" : s.substr(start);
    }

    std::string Parser::rtrim(const std::string &s) {
        const std::string WHITESPACE = " \n\r\t\f\v";
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    std::string Parser::trim(const std::string &s) {
        return rtrim(ltrim(s));
    }

    std::string Parser::exec(const char* cmd) {
        std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
        if (!pipe) return "ERROR";
        char buffer[128];
        std::string result = "";
        while (!feof(pipe.get())) {
            if (fgets(buffer, 128, pipe.get()) != NULL)
                result += buffer;
        }
        return result;
    }

    Parser Parser::read_from_file(const std::string &syfco_location, const std::string &filename) {
        Parser parser;

        // check whether the path is absolute or relative
        std::filesystem::path syfco_bin_path_obj(syfco_location);
        std::string syfco_bin_path = syfco_bin_path_obj.is_absolute()? syfco_location : "./" + syfco_location;

        std::string cmd_get_formula = syfco_bin_path + " --format ltlxba-fin -m fully "+filename;
        std::string formula_str = parser.exec(cmd_get_formula.c_str());
        parser.formula = parser.trim(formula_str);

        std::string cmd_get_ins = syfco_bin_path + " --format ltlxba-fin --print-input-signals "+filename;
        std::string ins_str = parser.exec(cmd_get_ins.c_str());
        std::string ins_str_trimmed = parser.trim(ins_str);
        ins_str_trimmed.erase(std::remove_if(ins_str_trimmed.begin(), ins_str_trimmed.end(), ::isspace),
                              ins_str_trimmed.end());
        std::vector<std::string> input_substr;
        boost::split(input_substr, ins_str_trimmed, boost::is_any_of(","));
        parser.input_variables = input_substr;

        std::string cmd_get_outs = syfco_bin_path + " --format ltlxba-fin --print-output-signals "+filename;
        std::string outs_str = parser.exec(cmd_get_outs.c_str());
        std::string outs_str_trimmed = parser.trim(outs_str);
        outs_str_trimmed.erase(std::remove_if(outs_str_trimmed.begin(), outs_str_trimmed.end(), ::isspace),
                               outs_str_trimmed.end());
        std::vector<std::string> output_substr;
        boost::split(output_substr, outs_str_trimmed, boost::is_any_of(","));
        parser.output_variables = output_substr;

        std::string cmd_get_target = syfco_bin_path + " --format ltlxba-fin -g "+filename;
        std::string target_str = parser.exec(cmd_get_target.c_str());
        std::string target_str_trimmed = parser.trim(target_str);
        target_str_trimmed.erase(std::remove_if(target_str_trimmed.begin(), target_str_trimmed.end(), ::isspace),
                               target_str_trimmed.end());
        parser.sys_first = (target_str_trimmed == "Moore")?  true : false;
        return parser;

    }

    std::vector<std::string> Parser::get_input_variables() const{
        return input_variables;
    }

    std::vector<std::string> Parser::get_output_variables() const{
        return output_variables;
    }

    std::string Parser::get_formula() const{
        return formula;
    }

    bool Parser::get_sys_first() const{
        return sys_first;
    }
}

