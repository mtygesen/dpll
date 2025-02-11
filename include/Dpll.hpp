#pragma once

#include <vector>
#include <string>
#include <algorithm>

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
            const std::string variableName;
            const bool isNegated;
    };
    
    class Clause {
        public:
            explicit Clause(std::vector<Variable> variables) : variables(std::move(variables)) {}
    
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
    
        private:
            std::vector<Variable> variables;
    };
    
    class Formula {
        public:
            explicit Formula(std::vector<Clause> clauses) : clauses(std::move(clauses)) {}
    
            bool isEmpty() const {
                return clauses.empty();
            }
    
            bool isEmptyClause() const {
                return clauses.size() == 1 && clauses[0].isEmpty();
            }
    
            void computeUnitClauses() {
                unitClauseIndicies.clear();
                for (size_t i = 0; i < clauses.size(); ++i) {
                    unitClauseIndicies.emplace_back(i);
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
        
            bool hasUnitClause() const {
                return unitClauseIndicies.empty();
            }
    
            bool hasPureVariable() const {
                return !pureVariables.empty();
            }
    
            void add(const Clause clause) {
                clauses.emplace_back(clause);
            }
    
            std::vector<Clause> &getClauses() {
                return clauses;
            }
    
            void removeClause(size_t idx) {
                if (idx < clauses.size()) {
                    clauses.erase(clauses.begin() + idx);
                }
            }
    
            const Variable &getFirstVar() const {
                return clauses[0].getVariables()[0];
            }

            const std::vector<size_t> &getUnitClauseIndicies() const {
                return unitClauseIndicies;
            }

            const auto &getPureVariables() const {
                return pureVariables;
            }
    
        private:
            std::vector<Clause> clauses;
            std::vector<size_t> unitClauseIndicies;
            std::unordered_map<std::string_view, std::pair<bool, bool>> pureVariables;
    };
    
    class Dpll {
        public:
            bool solve(Formula &formula) const {
                simplify(formula);
    
                if (formula.isEmpty()) return true;
                if (formula.isEmptyClause()) return false;
    
                const auto &var = formula.getFirstVar();
                
                Formula positiveSplit = formula;
            }
    
            void simplify(Formula &formula) const {
                formula.computeUnitClauses();
                while (formula.hasUnitClause()) {
                    unitPropagate(formula);
                }
    
                formula.computePureVariables();
                while (formula.hasPureVariable()) {
                    pureLiteralAssign(formula);
                }
            }
    
            void unitPropagate(Formula &formula) const {
                for (const size_t unitClauseIdx : formula.getUnitClauseIndicies()) {
                    const auto &unitClause = formula.getClauses()[unitClauseIdx];
                    const auto &unitVar = unitClause.getVariables()[0];
                    formula.removeClause(unitClauseIdx);

                    for (size_t i = 0; i < formula.getClauses().size(); ++i) {
                        const auto &clause = formula.getClauses()[i];
                        for (const auto &var : clause.getVariables()) {
                            if (var.getName() == unitVar.getName()) {
                                if (var.getIsNegated() == unitVar.getIsNegated()) {
                                    formula.removeClause(i--);
                                    break;
                                } else {
                                    formula.getClauses()[i].removeVariable(var);
                                    if (formula.getClauses()[i].isEmpty()) {
                                        formula.removeClause(i--);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
    
            void pureLiteralAssign(Formula &formula) const {
                const auto &pureVars = formula.getPureVariables();
                for (size_t i = 0; i < formula.getClauses().size(); ++i) {
                    const auto &clause = formula.getClauses()[i];
                    for (const auto& var : clause.getVariables()) {
                        auto it = pureVars.find(var.getName());
                        if (it == pureVars.end()) continue;
                        
                        const auto &occurrence = it->second;
                        if ((occurrence.first && !occurrence.second && !var.getIsNegated()) || 
                            (!occurrence.first && occurrence.second && var.getIsNegated())) {
                            formula.removeClause(i--);
                            break;
                        }
                    }
                }
            }
    };
}