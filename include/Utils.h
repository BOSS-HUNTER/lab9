#pragma once

#include <string>
#include <fstream>
#include <queue>
#include <tuple>
#include <boost/lockfree/stack.hpp>
#include <boost/asio/thread_pool.hpp>

#include "core/Parser.h"
#include "core/Downloader.h"
#include "CrawlerData.h"
#include "Page.h"

struct ThreadData
{
    using ImageContainer = std::queue<std::string>;
    using ThreadPool = boost::asio::thread_pool;

    Downloader &downloader;
    ThreadPool &downloaders;
    ThreadPool &parsers;
    ImageContainer &imageContainer;
    std::mutex &globalMutex;
    std::atomic<size_t> &parserAmount;
    std::ofstream &outputFile;

    std::mutex containerMutex{};
};

int programArguments(int argc, char *argv[]);

std::tuple<std::string, std::string> divideIntoHostAndTarget(const std::string &string);

void writeListToFile(const Parser::LinkContainer &images, ThreadData &data);

void afterParse(const Page &page, ThreadData &data);

void parse(const PageDownloaded &page, ThreadData &data);

void afterDownload(const PageDownloaded &page, ThreadData &data);

void download(const Page &page, ThreadData &data);
