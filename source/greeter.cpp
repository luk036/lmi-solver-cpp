#include <fmt/format.h>         // for format
#include <lmisolver/greeter.h>  // for LmiSolver, LanguageCode, LanguageCode...

// #include <__config>     // for std
#include <iosfwd>       // for string
#include <string>       // for basic_string
#include <type_traits>  // for move

using namespace lmisolver;
using namespace std;

LmiSolver::LmiSolver(string _name) : name(move(_name)) {}

auto LmiSolver::greet(LanguageCode lang) const -> std::string {
    switch (lang) {
        default:
        case LanguageCode::EN:
            return fmt::format("Hello, {}!", name);
        case LanguageCode::DE:
            return fmt::format("Hallo {}!", name);
        case LanguageCode::ES:
            return fmt::format("¡Hola {}!", name);
        case LanguageCode::FR:
            return fmt::format("Bonjour {}!", name);
    }
}
