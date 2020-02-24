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
