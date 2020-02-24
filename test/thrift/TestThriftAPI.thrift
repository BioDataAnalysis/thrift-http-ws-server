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

// Set namespace (or package, or prefix) for target languages:
namespace cpp TestThriftAPI
namespace java TestThriftAPI

// The following std_runtime_error is essential for BioDataAnalysis usage.
// It must be defined as exception for all methods in the API, otherwise
// the server and/or client may crash. The std_runtime_error is used as a
// container to pass std::exception from server to client. Please ensure
// that the method signature for all API methods is
// ... throws (1:std_runtime_error _std_runtime_error, ...);
exception std_runtime_error { 1: string _what }
// Other exceptions:
exception CustomException { 1: string mMessage, 2: string mData }

// Defining the services:
service TestThriftAPI {
    // This is a connection test helper and should be present in all APIs:
    i32 ping(1:i32 aTestValue) throws (1:std_runtime_error _std_runtime_error);

    // Test that the API can successfully pass exceptions defined in this API
    void triggerCustomException() throws (1:std_runtime_error _std_runtime_error, 2:CustomException aCustomException);

    // Test that the API can successfully pass the std_runtime_error
    void triggerAPIException() throws (1:std_runtime_error _std_runtime_error);

    // Test that the Server/Client can cope with arbitrary std::exceptions
    void triggerServerException() throws (1:std_runtime_error _std_runtime_error);

    // Benchmark method, send a data block of given size 10^aDataSizeIdx
    binary fetchData(1:i64 aDataSizeIdx) throws (1:std_runtime_error _std_runtime_error);
}
