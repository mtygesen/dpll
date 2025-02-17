# Davis–Putnam–Logemann–Loveland (DPLL) algorithm
DPLL implementation as a single-header include in C++20

## Example Usage
All formulas must be encoded in conjunctive normal form (CNF)

E.g. to solve (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r) ∧ (p ∨ ¬s) ∧ (q ∨ s) ∧ (r ∨ s):
```cpp
#include <dpll.hpp>

using namespace dpll;

int main(void) {
    dpll::Formula formula({
        {"p", "q"},     
        {"!p", "r"},    
        {"!q", "!r"},    
        {"p", "!s"},       
        {"q", "s"},       
        {"r", "s"}         
    });

    Solver::solve(formula);
}
```

Which yields the following output:
```
Starting solver...
Formula: (p ∨ q) ∧ (¬p ∨ r) ∧ (¬q ∨ ¬r) ∧ (p ∨ ¬s) ∧ (q ∨ s) ∧ (r ∨ s)
Solver finished in 0.004ms
Formula is satisfiable!
Assignment:
s ↦ ⊤
r ↦ ⊤
q ↦ ⊥
p ↦ ⊤
```

### Arguments
| Parameter | Description | Default |
|-----------|-------------|---------|
| `formula` | Formula to find satisfying assignment for | Required |
| `useUnitProp` | Whether to use unit propagation | `true` |
| `usePureAssign` | Whether to use pure literal assignment | `true` |
| `silent` | Whether to avoid printing to stdout | `false` |
