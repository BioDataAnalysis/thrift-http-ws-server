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
