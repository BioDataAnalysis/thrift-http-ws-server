//
// Developed for BioDataAnalysis GmbH <info@biodataanalysis.de>
//               Balanstrasse 43, 81669 Munich
//               https://www.biodataanalysis.de/
//
// Copyright (c) BioDataAnalysis GmbH. All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are not permitted. All information contained herein
// is, and remains the property of BioDataAnalysis GmbH.
// Dissemination of this information or reproduction of this material
// is strictly forbidden unless prior written permission is obtained
// from BioDataAnalysis GmbH.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// This file is based on boost::beast example: Advanced server, flex (plain + SSL)
// Official repository: https://github.com/boostorg/beast
//

#ifndef THRIFTHTTPSERVER_HH
#define THRIFTHTTPSERVER_HH

#include "bda/ThriftHelper.hh"

#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// forward declarations:
namespace apache {
namespace thrift {
class TProcessor;
namespace protocol {
class TProtocolFactory;
}
}
}
namespace boost {
namespace asio {
class io_context;
namespace ssl {
class context;
}
}
}
namespace bda {
class HTTPConnectListener;
}

namespace bda {

class ThriftHTTPWSServer {
public:
    ThriftHTTPWSServer(const std::string& aServerURL, const unsigned short aPort,
                     const std::string& aHTTPDocumentRoot, const int aThreads,
                     std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor,
                     const bda::ProtocolType aProtocolType);
    virtual ~ThriftHTTPWSServer() = default;

    /**
     * @brief Start the server in the background. This is a non-blocking
     * method that will perform the actual start asynchronously in the
     * background.
     */
    void asyncRun();

    /**
     * @brief Stop the server and block future connections. This method will
     * only exit after the server threads and workers completely shut down.
     */
    void stop();

protected:
    /**
     * @brief Load a signed certificate into the ssl context, and configure
     * the context for use with a server.
     */
    static void load_server_certificate(boost::asio::ssl::context& aSSLContext);

private:
    /**
     * @brief This internal method should be called inside its own thread.
     * It will start the server and all workers in the background without
     * blocking the main thread.
     */
    void backgroundRun();


    const int mThreads = 0;
    std::shared_ptr<std::thread> mMainServerThread;
    std::vector<std::thread> mWebServerThreads;

    std::shared_ptr<bda::HTTPConnectListener> mConnectionListener = nullptr;
    std::shared_ptr<boost::asio::io_context> mIOContext = nullptr;
};

}

#endif
