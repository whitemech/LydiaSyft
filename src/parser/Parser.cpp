//
// Created by shuzhu on 19/05/23.
//

#include "Parser.h"
#include "string_utilities.h"

#include <fstream>
#include <filesystem>
#include <algorithm>


namespace Syft {

    Parser::Parser()
    {}

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
        parser.formula = Syft::trim(formula_str);

        std::string cmd_get_ins = syfco_bin_path + " --format ltlxba-fin --print-input-signals "+filename;
        std::string ins_str = parser.exec(cmd_get_ins.c_str());
        std::string ins_str_trimmed = Syft::trim(ins_str);
        ins_str_trimmed.erase(std::remove_if(ins_str_trimmed.begin(), ins_str_trimmed.end(), ::isspace),
                              ins_str_trimmed.end());
        std::vector<std::string> input_substr;
        input_substr = split(ins_str_trimmed, ",");
        parser.input_variables = input_substr;
         
        std::ifstream in(filename);
        if (in.is_open()) {
            std::string line;
            bool inside_outputs = false;
            bool inside_guarantees = false;
            
            parser.agent_variables.clear();
            parser.agent_formulas.clear();
            while (std::getline(in, line)) {
                std::string trimmed = Syft::trim(line);
                size_t comment_pos = trimmed.find("//");
                if (comment_pos != std::string::npos) {
                    trimmed = trimmed.substr(0, comment_pos);
                }
                trimmed = Syft::trim(trimmed);
                if (trimmed.empty()) continue;
                // 1.OUTPUTS
                if (trimmed.find("OUTPUTS") != std::string::npos) {
                    inside_outputs = true;
                    inside_guarantees = false;
                    continue; 
                }
                
                // 2. GUARANTEES
                if (trimmed.find("GUARANTEES") != std::string::npos) {
                    inside_guarantees = true;
                    inside_outputs = false;
                    continue;
                }
                
                if (inside_outputs) {
                    if (trimmed.find("}") != std::string::npos) { inside_outputs = false; continue; }
                    //Trimm
                    std::replace(trimmed.begin(), trimmed.end(), ';', ' ');
                    std::vector<std::string> vars = split(trimmed, " ");
                    std::vector<std::string> clean_vars;
                    for(auto& v : vars) {
                        std::string v_c = Syft::trim(v);
                        if(!v_c.empty() && v_c != "{" && v_c != "}") clean_vars.push_back(v_c);
                    }
                    if(!clean_vars.empty()) parser.agent_variables.push_back(clean_vars);
                }
                else if (inside_guarantees) {
                    if (trimmed.find("}") != std::string::npos) { inside_guarantees = false; continue; }
                    size_t semi = trimmed.find(";");
                    if (semi != std::string::npos) trimmed = trimmed.substr(0, semi);
                    std::string clean_f = Syft::trim(trimmed);
                    
                    if(!clean_f.empty() && clean_f != "{" && clean_f != "}") {
                        parser.agent_formulas.push_back(clean_f);
                    }
                }
            }
            in.close();
        }


        // Fallback: we uso syco standard
        if (parser.agent_variables.empty()) {
            std::string cmd_get_outs = syfco_bin_path + " --format ltlxba-fin --print-output-signals "+filename;
            std::string outs_str = parser.exec(cmd_get_outs.c_str());
            std::string outs_str_trimmed = Syft::trim(outs_str);
            outs_str_trimmed.erase(std::remove_if(outs_str_trimmed.begin(), outs_str_trimmed.end(), ::isspace),
                                   outs_str_trimmed.end());
            std::vector<std::string> single_agent_vars = split(outs_str_trimmed, ",");
            parser.agent_variables.push_back(single_agent_vars);
        }

        
        std::string cmd_get_target = syfco_bin_path + " --format ltlxba-fin -g "+filename;
        std::string target_str = parser.exec(cmd_get_target.c_str());
        std::string target_str_trimmed = Syft::trim(target_str);
        target_str_trimmed.erase(std::remove_if(target_str_trimmed.begin(), target_str_trimmed.end(), ::isspace),
                               target_str_trimmed.end());
        parser.sys_first = (target_str_trimmed == "Moore");
        return parser;

    }

    std::vector<std::string> Parser::get_input_variables() const{
        return input_variables;
    }

    std::vector<std::vector<std::string>> Parser::get_agent_variables() const{
        return agent_variables;
    }

    std::string Parser::get_formula() const{
        return formula;
    }

    std::vector<std::string> Parser::get_agent_formulas() const{
        return agent_formulas;
    }

    bool Parser::get_sys_first() const{
        return sys_first;
    }

    //backword compatibility 
    std::vector<std::string> Parser::get_output_variables() const{
        if(agent_variables.empty()) return {};
        return agent_variables[0];
    }
}

