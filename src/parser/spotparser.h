#ifndef SPOTPARSER_H
#define SPOTPARSER_H

#include <spot/tl/formula.hh>

#include <vector>
#include <string>

using namespace spot;
using namespace std;

/**
Use spot parser to parse an LTL formula stored in a file
**/
formula parse_formula(string filename);

/**
check whether the input LTL formula is a safety LTL in negation normal form
**/
bool ensure_safey_ltl(formula& f);

/**
decompose the LTL formula to a list of subformulas
note here that subformulas will be negated and translated into nnf before return
**/
void decompose_formula(vector<formula>& list, formula f);

/**
decompose the LTL formula to a list of strings of subformulas in negation normal form
**/
void decompose_formula(vector<string>& formula_strs, formula f);

/**
push the Not operator in front of aps
*/
formula push_not_in(formula &f);
/**
compute the the negation normal form of an input LTL formula 
*/
formula get_nnf(formula &f);

/**
collect the set of props of a formula
*/
std::vector<std::string> get_props(formula &f);
#endif