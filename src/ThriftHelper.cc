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

#include "bda/ThriftHelper.hh"
#include "bda/ThriftAccessManager.hh"

#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/protocol/TProtocolException.h>
#include <thrift/transport/TSSLServerSocket.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportException.h>
#include <thrift/transport/TTransportUtils.h>

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace bda {

std::shared_ptr<apache::thrift::protocol::TProtocolFactory> createProtocolFactory(const bda::ProtocolType aProtocolType) {
    switch (aProtocolType) {
        case bda::ProtocolType::BINARY: {
            // The maximum number of bytes to read from the transport for variable-
            // length fields (such as strings or binary). C++ uses 0 for unlimited,
            // whereas Java uses -1 for unlimited?
            const int32_t string_limit = 0;

            // The maximum number of bytes to read from the network for containers
            // (like maps, sets, lists). C++ uses 0 for unlimited, whereas Java
            // uses -1 for unlimited?
            const int32_t container_limit = 0;

            // Set this to require the version number in the incoming message:
            const bool strict_read = false;
            // Set this to always write a version number in the outgoing message:
            const bool strict_write = true;

            return std::make_shared<apache::thrift::protocol::TBinaryProtocolFactory>(string_limit, container_limit, strict_read, strict_write);
        }
        case bda::ProtocolType::JSON: {
            return std::make_shared<apache::thrift::protocol::TJSONProtocolFactory>();
        }
        default:
            throw(std::runtime_error("bda::ThriftHTTPWSServer::createProtocolFactory(): ProtocolType not understood"));
    }
}

}
