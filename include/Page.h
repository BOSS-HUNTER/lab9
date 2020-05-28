#pragma once

#include "core/Parser.h"

struct Page
{
    std::string target;
    size_t depth;
};

struct PageParsed: public Page
{
    Parser::LinkContainer children;
    Parser::LinkContainer images;
};

struct PageDownloaded: public Page
{
    std::string html;
};