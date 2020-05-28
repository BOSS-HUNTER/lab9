#pragma once

#include <string>

struct CrawlerData
{
    static std::string url;
    static size_t depth;
    static size_t networkThreads;
    static size_t parserThreads;
    static std::string output;
};