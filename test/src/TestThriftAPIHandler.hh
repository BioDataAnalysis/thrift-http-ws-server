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
