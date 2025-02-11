# Davis–Putnam–Logemann–Loveland (DPLL) algorithm
DPLL single-header include in C++20

## Example Usage
```cpp
#include <Dpll.hpp>

int main(void) {
    dpll::Formula formula({{"p", "q"}, {"!p", "r"}, {"!q", "!r"}});  
    dpll::Dpll solver;
    solver.solve(formula);
}
```