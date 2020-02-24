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

#include "TestThriftAPIHandler.hh"

#include "bda/ThriftHelper.hh"

#include <bda/Helpers.hh>

TestThriftAPIHandler::TestThriftAPIHandler() {
    for (size_t vIdx = 0; vIdx < 8; ++vIdx) {
        const size_t vDataSize = static_cast<size_t>(std::pow(10.0, static_cast<double>(vIdx)) + 0.5);
        mData.emplace_back(vDataSize, 'a');
    }
}

int32_t TestThriftAPIHandler::ping(const int32_t aTestValue) {
    BDAMessage(8, "TestThriftAPIHandler::ping() called.\n");

    // NOTE: This method is available without login, as a connection test
    return ~aTestValue;
}

void TestThriftAPIHandler::fetchData(std::string& aData, const int64_t aDataSizeIdx) {
    BDAMessage(10, "TestThriftAPIHandler::fetchData(" + std::to_string(aDataSizeIdx) + ") called.\n");
    if (aDataSizeIdx < 0 || static_cast<size_t>(aDataSizeIdx) >= mData.size()) {
        throw(std::runtime_error("fetchData(): Data size index " + std::to_string(aDataSizeIdx) + " not defined."));
    }
    aData = mData[aDataSizeIdx];
}

void TestThriftAPIHandler::triggerCustomException() {
    throw(bda::generateThriftException<TestThriftAPI::CustomException>("TestThriftAPIHandler::triggerCustomException(): Throwing a TestThriftAPI::CustomException() as expected"));
}

void TestThriftAPIHandler::triggerAPIException() {
    // NOTE: This type is planned to be an internal type and
    // not meant to be constructed via our helper method:
    TestThriftAPI::std_runtime_error vInternalRuntimeError;
    vInternalRuntimeError._what = "TestThriftAPIHandler::triggerAPIException(): Throwing a TestThriftAPI::std_runtime_error() as expected";
    throw(vInternalRuntimeError);
}

void TestThriftAPIHandler::triggerServerException() {
    throw(std::runtime_error("TestThriftAPIHandler::triggerServerException(): Throwing a std::runtime_error() as expected"));
}
