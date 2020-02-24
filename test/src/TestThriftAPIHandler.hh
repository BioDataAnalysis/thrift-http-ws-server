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

#ifndef TESTTHRIFTAPIHANDLER_HH
#define TESTTHRIFTAPIHANDLER_HH

#include "TestThriftAPI.h"

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

class TestThriftAPIHandler : public TestThriftAPI::TestThriftAPIIf {
public:
    TestThriftAPIHandler();
    virtual ~TestThriftAPIHandler() = default;

    /** @brief Connection test helper method. */
    int32_t ping(const int32_t aTestValue) override;

    /** @brief Benchmark method, send a data block of given size 10^aDataSizeIdx */
    void fetchData(std::string& aData, const int64_t aDataSizeIdx) override;

    /** @brief Always throws a TestThriftAPI::CustomException. */
    void triggerCustomException() override;

    /** @brief Always throws a TestThriftAPI::std_runtime_error. */
    void triggerAPIException() override;

    /** @brief Always throws a std::runtime_error. */
    void triggerServerException() override;

protected:
    // blocks of random data of different size:
    std::vector<std::string> mData;
};

#endif
