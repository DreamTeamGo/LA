// Copyright 2020 <DreamTeamGo>
#pragma once

#include <iostream>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks.hpp>

#include <DBHashCreator.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

namespace logs {
void logInFile() {
    logging::add_file_log
            (
                    keywords::file_name = "/log/info.log",
                    keywords::rotation_size = 256 * 1024 * 1024,
                    keywords::time_based_rotation =
                            sinks::file::rotation_at_time_point(0, 0, 0),
                    keywords::filter =
                            logging::trivial::severity
                            >= logging::trivial::info,
                    keywords::format =
                            (
                                    expr::stream
                                            << boost::posix_time
                                            ::second_clock::local_time()
                                            << " : <" << logging
                                            ::trivial::severity
                                            << "> " << expr::smessage));

    logging::add_file_log
            (
                    keywords::file_name = "/log/trace.log",
                    keywords::rotation_size = 256 * 1024 * 1024,
                    keywords::time_based_rotation = sinks::file
                    ::rotation_at_time_point(0, 0, 0),
                    keywords::filter = logging::trivial::severity
                                       >= logging::trivial::trace,
                    keywords::format =
                            (
                                    expr::stream
                                            << boost::posix_time
                                            ::second_clock::local_time()
                                            << " : <" << logging::
                                            trivial::severity
                                            << "> " << expr::smessage));
}

void logInfo(const std::string &key, const std::string &hash) {
	std::cout << "key: " << key << " hash: " << hash << std::endl;
    BOOST_LOG_TRIVIAL(info) << "Thread with ID: "
                            << std::this_thread::get_id() << " Key: "
                            << key << " Hash: " << hash << std::endl;
}

void logTrace(const std::string &key, const std::string &value) {
	std::cout << "key: " << key << " value: " << value << std::endl;
    BOOST_LOG_TRIVIAL(trace) << "Thread with ID: "
                             << std::this_thread::get_id() << " Key: "
                             << key << " Value: " << value << std::endl;
}
}
