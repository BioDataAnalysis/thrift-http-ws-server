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
