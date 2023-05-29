//
// Created by shuzhu on 18/05/23.
//

#ifndef LYDIASYFT_APARSER_H
#define LYDIASYFT_APARSER_H

#include <stdexcept>
#include <vector>

namespace Syft {

/**
 * \brief A parser for reading LTLf synthesis benchmarks in TLSF format.
 */
    class Parser {
    private:

        std::vector<std::string> input_variables;
        std::vector<std::string> output_variables;
        std::string formula;
        bool sys_first;

        std::string exec(const char* cmd);
        std::string ltrim(const std::string &s);

        std::string rtrim(const std::string &s);

        std::string trim(const std::string &s);

    public:

        /**
        * \brief Creates a parser with no items.
        */
        Parser();

        /**
         * \brief Obtain an LTLf formula and construct a partition from a TLSF file.
         * \param filename The name of the TLSF file.
         */
        static Parser read_from_file(const std::string &filename);

        /**
         * \brief Return input variables in a vector.
         */
        std::vector<std::string> get_input_variables() const;

        /**
         * \brief Return output variables in a vector.
         */
        std::vector<std::string> get_output_variables() const;

        /**
         * \brief Return the formula.
         */
        std::string get_formula() const;

        /**
         * \brief Return true if the target is a Moore machine.
         */
        bool get_sys_first() const;
    };

}

#endif //LYDIASYFT_APARSER_H
