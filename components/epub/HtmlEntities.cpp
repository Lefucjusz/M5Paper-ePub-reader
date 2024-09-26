#include "HtmlEntities.hpp"

auto HtmlEntities::substitute(const std::string &input) -> std::string
{
    std::string output;
    output.reserve(input.size());

    /* Process each char */
    for (auto i = 0; i < input.size(); ++i) {
        const auto currentChar = input[i];

        /* Possible entity start */
        if (currentChar == entityStartMarker) {
            const auto entityLength = findEntityLength(input, i);

            if (entityLength == invalidEntityLength) {
                output += currentChar;
            }
            else {
                const auto &entity = input.substr(i, entityLength);
                const auto &replacement = lookupEntity(entity);
                output += replacement;
                i += entityLength - 1; // Skip the entity in input string
            }
        }
        else {
            output += currentChar;
        }
    }

    return output;
}

auto HtmlEntities::findEntityLength(const std::string &input, std::size_t offset) -> std::size_t
{
    std::size_t index = offset;
    while (true) {
        /* Index not found */
        if ((index >= input.size()) || ((index - offset) > maxEntityLength)) {
            return invalidEntityLength;
        }

        /* Index found, return its length */
        if (input[index] == entityEndMarker) {
            return (index - offset) + 1;
        }

        /* Check next index */
        ++index;
    }
}

auto HtmlEntities::lookupEntity(const std::string &entity) -> std::string
{
    const auto it = entitiesMap.find(entity);
    if (it == entitiesMap.end()) {
        return entity; // Not found, return entity itself
    }
    return it->second;
}
