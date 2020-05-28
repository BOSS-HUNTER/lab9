#include <queue>
#include <boost/bind.hpp>

#include "core/LogSetup.h"
#include "CrawlerData.h"
#include "Page.h"
#include "Utils.h"

int main(int argc, char *argv[])
{
    using ThreadPool = ThreadData::ThreadPool;

    LogSetup::init();
    BOOST_LOG_TRIVIAL(debug) << "Log setup complete";

    if (auto returnValue = programArguments(argc, argv); returnValue != 0) {
        return returnValue;
    }

    auto&&[host, target] = divideIntoHostAndTarget(CrawlerData::url);
    BOOST_LOG_TRIVIAL(debug) << "Initial Host: " << host << " target:" << target;

    ThreadPool downloaders{CrawlerData::networkThreads};
    ThreadPool parsers{CrawlerData::parserThreads};
    BOOST_LOG_TRIVIAL(debug) << "Initialized thread pools";

    ThreadData::ImageContainer imageContainer;
    std::atomic<size_t> parserAmount{1};
    std::mutex globalMutex;
    Downloader downloader{host, "443"};
    std::ofstream outputFile{CrawlerData::output};

    ThreadData threadData{
        downloader,
        downloaders,
        parsers,
        imageContainer,
        globalMutex,
        parserAmount,
        outputFile,
    };

    globalMutex.lock();
    boost::asio::post(downloaders,
                      boost::bind(download,
                                  Page{target, CrawlerData::depth},
                                  std::ref(threadData)));

    globalMutex.lock();     // And wait until all parsers end

    BOOST_LOG_TRIVIAL(info) << "Complete";
}