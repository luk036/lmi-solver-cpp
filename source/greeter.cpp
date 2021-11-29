#include <bingo/greeter.h>
#include <fmt/format.h>

using namespace bingo;
using namespace std;

Bingo::Bingo(string _name) : name(move(_name)) {}

std::string Bingo::greet(LanguageCode lang) const {
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
