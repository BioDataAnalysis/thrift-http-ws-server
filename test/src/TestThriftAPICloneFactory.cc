//
// Developed by:  Mario Emmenlauer (mario@emmenlauer.de)
//                Balanstrasse 43, 81669 Munich
//                http://www.biodataanalysis.de/
//
// With contributions by:
//
//
// Copyright (c) 2014-2018, BioDataAnalysis GmbH
// All Rights Reserved.
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
