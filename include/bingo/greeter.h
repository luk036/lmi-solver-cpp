#pragma once

#include <string>

namespace bingo {

    /**  Language codes to be used with the Bingo class */
    enum class LanguageCode { EN, DE, ES, FR };

    /**
     * @brief A class for saying hello in multiple languages
     */
    class Bingo {
        std::string name;

      public:
        /**
         * @brief Creates a new bingo
         * @param name the name to greet
         */
        Bingo(std::string name);

        /**
         * @brief Creates a localized string containing the greeting
         * @param lang the language to greet in
         * @return a string containing the greeting
         */
        std::string greet(LanguageCode lang = LanguageCode::EN) const;
    };

}  // namespace bingo
