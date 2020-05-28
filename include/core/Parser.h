#pragma once

#include <gumbo.h>
#include <string>
#include <list>
#include <stdexcept>
#include <boost/unordered_set.hpp>

struct ParserSearchParams
{
    std::string attr;
    boost::unordered_set<decltype(GUMBO_TAG_A)> tags;
};

class Parser
{
public:
    using LinkContainer = boost::unordered_set<std::string>;

    explicit Parser(std::string html);

    [[nodiscard]] LinkContainer getLinks(const ParserSearchParams &params);

private:
    static void getLinks(const GumboNode *node, const ParserSearchParams &params, LinkContainer &container);

    std::string html_;
    GumboNode *root;
};