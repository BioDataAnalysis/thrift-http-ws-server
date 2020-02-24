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

#include <TestThriftAPICloneFactory.hh>

TestThriftAPI::TestThriftAPIIf* TestThriftAPICloneFactory::getHandler(const apache::thrift::TConnectionInfo& aTConnectionInfo) {
    std::shared_ptr<apache::thrift::transport::TSocket> vTSocket = std::dynamic_pointer_cast<apache::thrift::transport::TSocket>(aTConnectionInfo.transport);
    std::cout << "Incoming connection" << std::endl;
    std::cout << "\tSocketInfo: " << vTSocket->getSocketInfo() << std::endl;
    std::cout << "\tPeerHost: " << vTSocket->getPeerHost() << std::endl;
    std::cout << "\tPeerAddress: " << vTSocket->getPeerAddress() << std::endl;
    std::cout << "\tPeerPort: " << vTSocket->getPeerPort() << std::endl;
    return new TestThriftAPIHandler;
}

void TestThriftAPICloneFactory::releaseHandler(TestThriftAPI::TestThriftAPIIf* handler) {
    delete handler;
}
