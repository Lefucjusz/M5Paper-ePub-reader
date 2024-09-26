#pragma once

#include <string>
#include <map>
#include <limits>

class HtmlEntities
{
    public:
        auto substitute(const std::string &input) -> std::string;

    private:
        static constexpr auto maxEntityLength = 6;
        static constexpr auto invalidEntityLength = std::numeric_limits<std::size_t>::max();

        static constexpr auto entityStartMarker = '&';
        static constexpr auto entityEndMarker = ';';

        std::map<std::string, std::string> entitiesMap = {
            {"&quot;", "\""},
            {"&amp;", "&"},
            {"&lt;", "<"},
            {"&gt;", ">"},
            {"&nbsp;", " "}
        };

        auto findEntityLength(const std::string &input, std::size_t offset) -> std::size_t;
        auto lookupEntity(const std::string &entity) -> std::string;
};
