#include <iostream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/bind.hpp>

#include "Utils.h"
#include "CrawlerData.h"

std::tuple<std::string, std::string> divideIntoHostAndTarget(const std::string &string)
{
    std::string host = string;
    std::string target = "/";

    auto iterator = std::find(string.cbegin(), string.cend(), '/');
    if (iterator == string.cend()) {
        return {host, target};
    }

    host = std::string{string.cbegin(), iterator};
    target = std::string{iterator, string.cend()};

    return {host, target};
}

int programArguments(int argc, char **argv)
{
    namespace options = boost::program_options;

    options::options_description allOptions("Available options");
    allOptions.add_options()
        ("url",
         options::value<std::string>(&CrawlerData::url),
         "URL to web page")
        ("depth",
         options::value<size_t>(&CrawlerData::depth)->default_value(5),
         "Search depth")
        ("network_threads",
         options::value<size_t>(&CrawlerData::networkThreads)->default_value(2),
         "Downloader thread amount")
        ("parser_threads",
         options::value<size_t>(&CrawlerData::parserThreads)->default_value(2),
         "Parser thread amount")
        ("output",
         options::value<std::string>(&CrawlerData::output),
         "Path to output file")
        ("help", "Print help message");

    options::variables_map variablesMap;
    options::store(options::parse_command_line(argc, argv, allOptions), variablesMap);
    options::notify(variablesMap);

    if (variablesMap.count("help")) {
        std::cout << allOptions << "\n";
        return 1;
    }

    for (const std::string &argName: std::array<std::string, 2>{"url", "output"}) {
        if (!variablesMap.count(argName)) {
            std::cout << argName << " arg was not set.\nType --help to get information";
            return 1;
        }
    }

    return 0;
}

void writeListToFile(const Parser::LinkContainer &images, ThreadData &data)
{
    std::string result{};
    for (const auto &image : images) {
        result += image + "\n";
    }

    data.outputFile << result;
    BOOST_LOG_TRIVIAL(debug) << "Written to file " << CrawlerData::output;
}

void afterParse(const Page &page, ThreadData &data)
{
    boost::asio::post(data.downloaders,
                      boost::bind(download,
                                  page,
                                  std::ref(data)));
}

void parse(const PageDownloaded &page, ThreadData &data)
{
    Parser parser{page.html};
    auto images = parser.getLinks({
                                      "src",
                                      {GUMBO_TAG_IMG}
                                  });
    auto links = parser.getLinks({
                                     "href",
                                     {GUMBO_TAG_A}
                                 });

    BOOST_LOG_TRIVIAL(debug) << "Images: ";
    size_t amount = 0;
    for (const std::string &image : images) {
        amount++;
        BOOST_LOG_TRIVIAL(debug) << "    Image: " << image;

        std::lock_guard<std::mutex> locker(data.containerMutex);
        data.imageContainer.push(image);
    }
    BOOST_LOG_TRIVIAL(info) << "Found images on '" << page.target << "': " << amount;
    writeListToFile(images, data);

    data.parserAmount--;
    if (page.depth == 0) {
        BOOST_LOG_TRIVIAL(info) << "Parser amount: " << data.parserAmount;

        if (data.parserAmount == 0) {
            data.globalMutex.unlock();
        }
        return;
    }

    if (auto size = links.size(); size > 0) {
        data.parserAmount += size;
    }
    BOOST_LOG_TRIVIAL(info) << "Parser amount: " << data.parserAmount;
    BOOST_LOG_TRIVIAL(debug) << "Links: ";
    for (const std::string &link : links) {
        BOOST_LOG_TRIVIAL(debug) << "    Link: " << link;
        afterParse({link, page.depth - 1}, data);
    }

}

void afterDownload(const PageDownloaded &page, ThreadData &data)
{
    boost::asio::post(data.parsers,
                      boost::bind(parse,
                                  page,
                                  std::ref(data)));
}

void download(const Page &page, ThreadData &data)
{
    Downloader downloader{data.downloader.getHost(),
                          data.downloader.getPort()};
    std::string html = downloader.download(page.target);
    PageDownloaded result{
        {page.target, page.depth},
        html
    };

    afterDownload(result, data);
}

