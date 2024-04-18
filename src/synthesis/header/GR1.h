//
// Created by shuzhu on 21/01/24.
//

#ifndef LYDIASYFT_GR1_H
#define LYDIASYFT_GR1_H

#include <fstream>
#include "string_utilities.h"
#include "VarMgr.h"

namespace Syft {
    /**
     * GR(1) file format is similar to the regular CNF format, such that every justice
     * is represented as a conjunction over disjunctive clauses.
     * Each justice is labeled with `env` or `agn` to refer to environment or
     * agent justice respectively, and ended with `end`.
     * Consider GR(1) formula
     * GF(p_1) & GF(p_1 & q_1) & ... & GF(!p_1 & (q_1 | q_2)) ->
     * GF(q_1) & GF(p_1 & q_1) & ... & GF(!q_1 & (p_1 | p_2)),
     * this formula is written as follows (variables in uppercase):
     * --Env-Justice--
     * P_1
     * --Justice-End--
     *
     * --Env-Justice--
     * P_1
     * Q_1
     * --Justice-End--
     *
     * --Env-Justice--
     * !P_1
     * Q_1 Q_2
     * --Justice-End--
     *
     * --Agn-Justice--
     * Q_1
     * --Justice-End--
     *
     * --Agn-Justice--
     * P_1
     * Q_1
     * --Justice-End--
     *
     * --Agn-Justice--
     * !Q_1
     * P_1 Q_1
     * --Justice-End--
     * End
     */

    struct GR1 {
        std::vector<CUDD::BDD> env_justices;
        std::vector<CUDD::BDD> agn_justices;

        /**
         * \brief Stores a player justice
         */
        static CUDD::BDD read_gr1_justice(const std::shared_ptr<VarMgr> &var_mgr,
                                          std::istream &in,
                                          std::size_t &line_number) {
            std::string line;
            CUDD::BDD justice = var_mgr->cudd_mgr()->bddOne();

            while (std::getline(in, line) && (line.find("Justice-End") == std::string::npos)) {

                CUDD::BDD clause = var_mgr->cudd_mgr()->bddZero();
                if (line == "true") {
                    clause = var_mgr->cudd_mgr()->bddOne();
                } else if (line == "false") {
                    clause = var_mgr->cudd_mgr()->bddZero();
                } else {
                    std::vector<std::string> tokens;
                    trim(line); // remove leading and trailing whitespace
                    tokens = split(line, " ");
                    std::vector<std::string> variable_names;

                    for (std::string token: tokens) {
                        std::string variable_name;
                        variable_name = (line.find("!") == std::string::npos) ? token : token.substr(2,
                                                                                                     token.size() - 3);
//                        variable_name = to_upper_copy(variable_name); // turn variable names into uppercase to match the variable names in InputOutputPartition
                        variable_names.push_back(variable_name);
                        var_mgr->create_named_variables(variable_names);
                        CUDD::BDD variable_bdd = var_mgr->name_to_variable(variable_name);
                        clause = (line.find("!") == std::string::npos) ? (clause | variable_bdd) : (clause |
                                                                                                    !variable_bdd);
                    }
                }
                justice = justice & clause;
                ++line_number;
            }
            return justice;
        }

        static std::runtime_error bad_file_format_exception(
                std::size_t line_number) {
            return std::runtime_error("Incorrect format in line " +
                                      std::to_string(line_number) +
                                      " of the GR(1) file.");
        }

        /**
         * \brief Stores a GR(1) condition.
         *
         * \param gr1_filename The name of the file to read the GR(1) condition from.
         * \return The GR(1) condition stored in the file
         */
        static GR1 read_from_gr1_file(const std::shared_ptr<VarMgr> &var_mgr,
                                      const std::string &gr1_filename) {
            GR1 gr1;
            std::ifstream in(gr1_filename);
            std::size_t line_number = 0;
            std::string line;
            while (std::getline(in, line) && line != "End") {
                if (line.find("Justice") != std::string::npos) {
                    if (line.find("Env") == std::string::npos && line.find("Agn") == std::string::npos) {
                        throw bad_file_format_exception(line_number);
                    }
                    ++line_number;
                    CUDD::BDD justice = read_gr1_justice(var_mgr, in, line_number);
                    if (line.find("Env") != std::string::npos) {
                        gr1.env_justices.push_back(justice);
                    } else {
                        gr1.agn_justices.push_back(justice);
                    }
                }
                ++line_number;
            }
            return gr1;
        }
    };


}

#endif //LYDIASYFT_GR1_H
