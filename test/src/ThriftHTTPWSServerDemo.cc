/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "bda/ThriftHTTPWSServer.hh"

#include <bda/Helpers.hh>

#include "TestThriftAPI.h"
#include "TestThriftAPIHandler.hh"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>

#include <boost/asio/io_context.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <string>


void ParseCommandLineArguments(boost::program_options::variables_map& aParsedCmdLineOptionsMap, std::vector<std::string>& aNonParsedCmdLineOptions, const int argc, char** const argv) {
    // Declare command line options.
    boost::program_options::options_description vCMDLineStdOptions("Allowed options");
    // clang-format off
    vCMDLineStdOptions.add_options()
        ("help,h",                                                                                                           "this help message")
        ("verbose,v",        boost::program_options::value<uint8_t>()->default_value(6),                                     "verbosity (higher numbers mean more verbose)")
        ("interface,i",      boost::program_options::value<std::string>()->default_value("0.0.0.0"),                         "network interface")
        ("port,p",           boost::program_options::value<uint16_t>()->default_value(9090),                                 "network port")
        ("http-directory,d", boost::program_options::value<std::string>(),                                                   "http document root directory")
        ("threads,t",        boost::program_options::value<uint8_t>()->default_value(8),                                     "number of threads")
        ("uptime-sec,u",     boost::program_options::value<uint32_t>()->default_value(std::numeric_limits<uint32_t>::max()), "automatic shutdown after (seconds)")
        ("logfile,l",        boost::program_options::value<std::string>(),                                                   "logfile (overwrites existing)");
    // clang-format on


    const auto vParsedCmdLineOptions = boost::program_options::command_line_parser(argc, argv).options(vCMDLineStdOptions).allow_unregistered().run();
    boost::program_options::store(vParsedCmdLineOptions, aParsedCmdLineOptionsMap);
    boost::program_options::notify(aParsedCmdLineOptionsMap);
    aNonParsedCmdLineOptions = boost::program_options::collect_unrecognized(vParsedCmdLineOptions.options, boost::program_options::include_positional);

    if (aParsedCmdLineOptionsMap.count("help")) {
        std::cout << vCMDLineStdOptions;
        std::exit(0);
    }
}

int main(int argc, char** argv) {
    // Parse command line options:
    boost::program_options::variables_map vParsedCmdLineOptionsMap;
    std::vector<std::string> vNonParsedCmdLineOptions;
    ParseCommandLineArguments(vParsedCmdLineOptionsMap, vNonParsedCmdLineOptions, argc, argv);

    // Validate the arguments:
    if (vNonParsedCmdLineOptions.size() > 0) {
        std::cerr << "ThriftHTTPWSServerDemo(): Received additional argument(s) " << vNonParsedCmdLineOptions.front() << std::endl;
        std::exit(1);
    } else if (vParsedCmdLineOptionsMap.count("http-directory") < 1) {
        std::cerr << "ThriftHTTPWSServerDemo(): Missing required argument --http-directory" << std::endl;
        std::exit(1);
    }

    BDAMessage(9, "ThriftHTTPWSServerDemo(): Received " + std::to_string(argc) + " command line arguments:\n");
    for (int vArgIdx = 0; vArgIdx < argc; ++vArgIdx) {
        BDAMessage(9, std::string(argv[vArgIdx]) + "\n");
    }


    // create the Thrift Processor that will handle the server API calls
    std::shared_ptr<TestThriftAPIHandler> vThriftHandler = std::make_shared<TestThriftAPIHandler>();
    std::shared_ptr<apache::thrift::TProcessor> vThriftProcessor = std::make_shared<TestThriftAPI::TestThriftAPIProcessor>(vThriftHandler);


    BDAMessage(2, "Demo: Will construct the webserver\n");
    const std::string vServerAddress = vParsedCmdLineOptionsMap["interface"].as<std::string>();
    const uint16_t ServerPort = vParsedCmdLineOptionsMap["port"].as<uint16_t>();
    const std::string vHTTPDocumentRoot = vParsedCmdLineOptionsMap["http-directory"].as<std::string>();
    const uint8_t vThreads = vParsedCmdLineOptionsMap["threads"].as<uint8_t>();
    bda::ThriftHTTPWSServer vThriftHTTPWSServer(vServerAddress, ServerPort, vHTTPDocumentRoot, vThreads,
                                            vThriftProcessor, bda::ProtocolType::BINARY);
    BDAMessage(2, "Demo: Webserver constructed\n");


    BDAMessage(2, "Demo: Will start the webserver\n");
    vThriftHTTPWSServer.asyncRun();
    BDAMessage(2, "Demo: Webserver started\n");


    // Sleep for a while, before shutting down the server
    std::this_thread::sleep_for(std::chrono::seconds(vParsedCmdLineOptionsMap["uptime-sec"].as<uint32_t>()));


    BDAMessage(2, "Demo: Will request the webserver to end\n");
    vThriftHTTPWSServer.stop();
    BDAMessage(2, "Demo: Webserver ended\n");

    return 0;
}
