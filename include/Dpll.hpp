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
    
        private:
            friend class Clause;

            const std::string variableName;
            const bool isNegated;
    };
    
    class Clause {  
        public:
            explicit Clause(std::vector<Variable> variables) : variables(std::move(variables)) {}

        private:
            friend class Dpll;
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

            void removeVariable(const Variable& variable) {
                std::vector<Variable> newVariables;
                for (const auto &var : variables) {
                    if (var != variable) {
                        newVariables.push_back(var);
                    }
                }

                variables = std::move(newVariables);
            }

            std::vector<Variable> variables;
    };
    
    class Formula {
        public:
            explicit Formula() = default;

            explicit Formula(std::initializer_list<std::initializer_list<const std::string>> clauses) {
                addClauses(clauses);
            }

            bool isEmpty() const {
                return clauses.empty();
            }
    
            bool hasEmptyClause() const {
                return !isEmpty() && std::any_of(clauses.begin(), clauses.end(), 
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
            friend class Dpll;

            const auto &getPureVariables() const {
                return pureVariables;
            }

            bool hasUnitClause() const {
                return !unitClauseIndicies.empty();
            }
    
            bool hasPureVariable() const {
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
    
    class Dpll {
        public:
            explicit Dpll() = default;

            using Assignment = std::unordered_map<std::string, bool>;

            void solve(Formula &formula, bool useSimplification = true) const {
                std::cout << "Starting solver..." << '\n';
                Assignment assignment;

                const auto start = std::chrono::high_resolution_clock::now();
                const bool satisfiable = solve(formula, assignment, useSimplification);
                const auto end = std::chrono::high_resolution_clock::now();

                const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "Solver finished in " << duration.count() << "ms\n";

                if (satisfiable) {
                    std::cout << "Formula is satisfiable!\n";
                    std::cout << "Assignment:\n";
                    for (const auto &[var, value] : assignment) {
                        std::cout << var << " = " << (value ? "true" : "false") << '\n';
                    }
                } else {
                    std::cout << "Formula is unsatisfiable!\n";
                }
            }

        private:
            bool solve(Formula &formula, Assignment &assignment, bool useSimplification) const {
                if (useSimplification) simplify(formula, assignment);
            
                if (formula.isEmpty()) return true;
                if (formula.hasEmptyClause()) return false;

                const auto &var = formula.getFirstVar();

                std::string varName(var.getName());
            
                Formula positiveSplit = formula;
                assignment[varName] = true;
                applyAssignment(positiveSplit, var, true);
                if (solve(positiveSplit, assignment, useSimplification)) return true;
                
                Formula negativeSplit = formula;
                assignment[varName] = false;
                applyAssignment(negativeSplit, var, false);
                return solve(negativeSplit, assignment, useSimplification);
            }

            void applyAssignment(Formula &formula, const Variable &var, bool value) const {
                std::string varName(var.getName());
                for (size_t i = 0; i < formula.getClauses().size(); ++i) {
                    auto &clause = formula.getClauses()[i];
                    bool shouldRemove = false;
                    
                    for (const auto &clauseVar : clause.getVariables()) {
                        if (clauseVar.getName() == var.getName() && clauseVar.getIsNegated() != value) {
                            shouldRemove = true;
                            break;
                        }
                    }
                    
                    if (shouldRemove) {
                        formula.removeClause(i--);
                        continue;
                    }

                    clause.removeVariable(Variable(varName, value));
                }
            }
    
            void simplify(Formula &formula, Assignment &assignment) const {
                formula.computeUnitClauses();
                while (formula.hasUnitClause()) {
                    unitPropagate(formula, assignment);
                }
    
                formula.computePureVariables();
                while (formula.hasPureVariable()) {
                    pureLiteralAssign(formula, assignment);
                }
            }
    
            void unitPropagate(Formula &formula, Assignment &assignment) const {
                const auto &unitIndices = formula.getUnitClauseIndicies();
                
                for (const size_t idx : unitIndices) {
                    if (idx >= formula.getClauses().size()) continue;
                    
                    const auto &clause = formula.getClauses()[idx];
                    if (clause.isEmpty()) continue;
                    
                    const auto &unitVar = clause.getVariables()[0];

                    std::string varName(unitVar.getName());
                    assignment[varName] = !unitVar.getIsNegated();
                    
                    for (size_t i = 0; i < formula.getClauses().size(); ++i) {
                        auto &currentClause = formula.getClauses()[i];
                        bool shouldRemove = false;
                        
                        for (const auto &var : currentClause.getVariables()) {
                            if (std::string(var.getName()) == std::string(unitVar.getName()) && 
                                var.getIsNegated() == unitVar.getIsNegated()) {
                                shouldRemove = true;
                                break;
                            }
                        }
                        
                        if (shouldRemove) {
                            formula.removeClause(i);
                            continue;
                        }
                        
                        std::string unitVarName(unitVar.getName());
                        Variable oppositeVar(unitVarName, !unitVar.getIsNegated());
                        currentClause.removeVariable(oppositeVar);
                    }
                }

                formula.computeUnitClauses();
            }
    
            void pureLiteralAssign(Formula &formula, Assignment &assignment) const {
                const auto &pureVars = formula.getPureVariables();
                
                for (const auto &[varName, occurrence] : pureVars) {
                    bool value = occurrence.first;
                    assignment[std::string(varName)] = value;
        
                    for (size_t i = 0; i < formula.getClauses().size(); ++i) {
                        auto &clause = formula.getClauses()[i];
                        bool shouldRemove = false;
            
                        for (const auto &var : clause.getVariables()) {
                            if (var.getName() == varName) {
                                shouldRemove = true;
                                break;
                            }
                        }
            
                        if (shouldRemove) {
                            formula.removeClause(i--);
                        }
                    }
                }
            
                formula.computePureVariables();
            }
    };
}