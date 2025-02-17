#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <cassert>
#include <chrono>

namespace dpll {
    class Variable {
        public:
            explicit Variable(std::string variableName, bool isNegated = false) : variableName(std::move(variableName)), isNegated(isNegated) {}

            Variable(const Variable &other) : variableName(other.variableName), isNegated(other.isNegated) {}

            Variable(Variable &&other) noexcept : variableName(std::move(const_cast<std::string&>(other.variableName))), isNegated(other.isNegated) {}
            
            Variable &operator=(Variable &&other) noexcept {
                const_cast<std::string&>(variableName) = std::move(const_cast<std::string&>(other.variableName));
                const_cast<bool&>(isNegated) = other.isNegated;
                return *this;
            }

            std::string_view getName() const {
                return variableName;
            }
    
            bool getIsNegated() const {
                return isNegated;
            }

            bool operator==(const Variable &other) const {
                return variableName == other.variableName && isNegated == other.isNegated;
            }

            bool operator!=(const Variable &other) const {
                return !(*this == other);
            }

            friend std::ostream &operator<<(std::ostream &os, const Variable &var) {
                return os << (var.isNegated ? "¬" : "") << var.variableName;
            }
    
        private:
            friend class Clause;

            const std::string variableName;
            const bool isNegated;
    };
    
    class Clause {  
        public:
            explicit Clause(std::vector<Variable> variables) : variables(std::move(variables)) {}

            friend std::ostream &operator<<(std::ostream &os, const Clause &clause) {
                os << "(";
                const auto &vars = clause.getVariables();
                for (size_t i = 0; i < vars.size(); ++i) {
                    os << vars[i];
                    if (i < vars.size() - 1) {
                        os << " ∨ "; 
                    }
                }

                return os << ")";
            }

        private:
            friend class Solver;
            friend class Formula;
            
            bool isEmpty() const {
                return variables.empty();
            }
    
            bool isUnit() const {
                return variables.size() == 1;
            }
    
            const std::vector<Variable> &getVariables() const {
                return variables;
            }  

            void removeVariable(const Variable &varToRemove) {
                variables.erase(
                    std::remove_if(variables.begin(), variables.end(),
                        [&varToRemove](const Variable &var) { return var == varToRemove; }),
                    variables.end()
                );
            }

            std::vector<Variable> variables;
    };
    
    class Formula {
        public:
            explicit Formula() = default;

            explicit Formula(std::initializer_list<std::initializer_list<const std::string>> clauses) {
                addClauses(clauses);
            }

            friend std::ostream &operator<<(std::ostream &os, const Formula &formula) {
                const auto &clauses = formula.clauses;
                for (size_t i = 0; i < clauses.size(); ++i) {
                    os << clauses[i];
                    if (i < clauses.size() - 1) {
                        os << " ∧ ";
                    }
                }

                return os;
            }

            bool isEmpty() const noexcept {
                return clauses.empty();
            }
    
            bool hasEmptyClause() const {
                return std::any_of(clauses.begin(), clauses.end(), 
                    [](const Clause &clause) { return clause.isEmpty(); });
            }
    
            std::vector<Clause> &getClauses() {
                return clauses;
            }

            void addClause(std::initializer_list<const std::string> variables) {
                std::vector<Variable> vars;
                for (const auto &var : variables) {
                    if (var.empty()) continue;
                    
                    bool isNegated = var[0] == '!';
                    std::string varName = isNegated ? var.substr(1) : var;
                    vars.emplace_back(varName, isNegated);
                }
                
                add(Clause(std::move(vars)));
            }

            void addClauses(std::initializer_list<std::initializer_list<const std::string>> clauses) {
                for (const auto &clause : clauses) {
                    addClause(clause);
                }
            }
    
        private:
            friend class Solver;

            const auto &getPureVariables() const {
                return pureVariables;
            }

            bool hasUnitClause() const noexcept {
                return !unitClauseIndicies.empty();
            }
    
            bool hasPureVariable() const noexcept {
                return !pureVariables.empty();
            }
    
            void add(const Clause clause) {
                clauses.emplace_back(clause);
            }

            void removeClause(size_t idx) {
                if (idx < clauses.size()) {
                    clauses.erase(clauses.begin() + idx);
                }
            }

            void computeUnitClauses() {
                unitClauseIndicies.clear();
                for (size_t i = 0; i < clauses.size(); ++i) {
                    if (clauses[i].isUnit()) {
                        unitClauseIndicies.emplace_back(i);
                    }
                }
            }

            void computePureVariables() {
                pureVariables.clear();
                for (const auto &clause : clauses) {
                    for (const auto &var : clause.getVariables()) {
                        auto &occurrence = pureVariables[var.getName()];
                        var.getIsNegated() ? occurrence.second = true : occurrence.first = true;
                    }
                }

                for (auto it = pureVariables.begin(); it != pureVariables.end();) {
                    if (it->second.first && it->second.second || 
                       (!it->second.first && !it->second.second)) {
                        it = pureVariables.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
    
            const Variable &getFirstVar() const {
                return clauses[0].getVariables()[0];
            }

            const std::vector<size_t> &getUnitClauseIndicies() const {
                return unitClauseIndicies;
            }

            std::vector<Clause> clauses;
            std::vector<size_t> unitClauseIndicies;
            std::unordered_map<std::string_view, std::pair<bool, bool>> pureVariables;
    };
    
    class Solver {
        public:
            explicit Solver() = default;

            using Assignment = std::unordered_map<std::string, bool>;

            static std::pair<bool, Assignment> solve(Formula &formula, bool useUnitProp = true, bool usePureAssign = true,  bool silent = false) {
                if (!silent) {
                    std::cout << "Starting solver..." << '\n';
                    std::cout << "Formula: " << formula << '\n';
                }

                Assignment assignment;

                const auto start = std::chrono::high_resolution_clock::now();
                const bool satisfiable = solve(formula, assignment, useUnitProp, usePureAssign);
                const auto end = std::chrono::high_resolution_clock::now();
        
                const auto result = std::make_pair(satisfiable, assignment);
        
                if (silent) return result;

                const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "Solver finished in " << duration.count() << "ms\n";

                if (satisfiable) {
                    std::cout << "Formula is satisfiable!\n";
                    if (!assignment.empty()) {
                        std::cout << "Assignment:\n";
                        for (const auto &[var, value] : assignment) {
                            std::cout << var << " ↦ " << (value ? "⊤" : "⊥") << '\n';
                        }
                    } else {
                        std::cout << "No variables to assign\n";
                    }
                } else {
                    std::cout << "Formula is unsatisfiable!\n";
                }

                return result;
            }

        private:
            static bool solve(Formula &formula, Assignment &assignment, bool useUnitProp, bool usePureAssign) {
                simplify(formula, assignment, useUnitProp, usePureAssign);
            
                if (formula.isEmpty()) return true;
                if (formula.hasEmptyClause()) return false;

                const auto &var = formula.getFirstVar();

                std::string varName(var.getName());
            
                Formula positiveSplit = formula;
                assignment[varName] = true;

                applyAssignment(positiveSplit, var, true);
                if (solve(positiveSplit, assignment, useUnitProp, usePureAssign)) return true;
                
                Formula negativeSplit = formula;
                assignment[varName] = false;
                applyAssignment(negativeSplit, var, false);
                return solve(negativeSplit, assignment, useUnitProp, usePureAssign);
            }

            static void applyAssignment(Formula &formula, const Variable &variable, bool value) {
                std::string varName(variable.getName());
                auto &clauses = formula.getClauses();
                for (size_t i = clauses.size(); i-- > 0;) {
                    auto &clause = clauses[i];    
                    const auto &variables = clause.getVariables();

                    const bool shouldRemove = std::any_of(variables.begin(), variables.end(), 
                                                    [&variable, &value](const Variable &var) { return variable == var && var.getIsNegated() != value; });
                    
                    if (shouldRemove) {
                        formula.removeClause(i);
                    } else {
                        clause.removeVariable(Variable(varName, value));
                    }
                }
            }
    
            static void simplify(Formula &formula, Assignment &assignment, bool useUnitProp, bool usePureAssign) {
                if (useUnitProp) {
                    formula.computeUnitClauses();
                    while (formula.hasUnitClause()) {
                        unitPropagate(formula, assignment);
                    }
                }

                if (usePureAssign) {
                    while (formula.hasPureVariable()) {
                        pureLiteralAssign(formula, assignment);
                    }
                }
            }
    
            static void unitPropagate(Formula &formula, Assignment &assignment) {
                auto &clauses = formula.getClauses();
                std::vector<size_t> unitIndices = formula.getUnitClauseIndicies();
                for (const size_t originalIdx : unitIndices) {
                    if (originalIdx >= clauses.size()) continue;
                    
                    const auto &clause = clauses[originalIdx];
                    if (!clause.isUnit()) continue;
                    
                    const auto &unitVar = clause.getVariables()[0];
                    std::string varName(unitVar.getName());
                    assignment[varName] = !unitVar.getIsNegated();
                    
                    for (size_t i = clauses.size(); i-- > 0;) {
                        auto &currentClause = clauses[i];
                        const auto &variables = currentClause.getVariables();
                        
                        const bool shouldRemove = std::any_of(variables.begin(), variables.end(), 
                                                        [&unitVar](const Variable &var) { return var == unitVar; });
                        
                        if (shouldRemove) {
                            formula.removeClause(i);
                        } else {
                            Variable oppositeVar(varName, !unitVar.getIsNegated());
                            currentClause.removeVariable(oppositeVar);
                        }
                    }
                }
                
                formula.computeUnitClauses();
            }
    
            static void pureLiteralAssign(Formula &formula, Assignment &assignment) {
                const auto &pureVars = formula.getPureVariables();
                for (const auto &[varName, occurrence] : pureVars) {
                    const bool value = occurrence.first;
                    assignment[std::string(varName)] = value;
            
                    for (size_t i = formula.getClauses().size(); i-- > 0;) {
                        auto &currentClause = formula.getClauses()[i];
                        const auto &variables = currentClause.getVariables();

                        const bool shouldRemove = std::any_of(variables.begin(), variables.end(), 
                                                        [&varName](const Variable &var) { return var.getName() == varName; });
                        
                        if (shouldRemove) formula.removeClause(i);
                    }
                }
            
                formula.computePureVariables();
            }
    };
}