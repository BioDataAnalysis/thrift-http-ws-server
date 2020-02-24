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

#ifndef THRIFTHELPER_HH
#define THRIFTHELPER_HH

#include <memory>
#include <string>

// forward declarations:
namespace apache {
namespace thrift {
class TProcessor;
namespace protocol {
class TProtocolFactory;
}
namespace transport {
class TSSLSocketFactory;
}
}
}

namespace bda {

enum class ProtocolType {
    BINARY,
    JSON
};

/**
 * @brief Generate a thrift exception in the typical setup for BioDataAnalysis
 * usage. The message can be passed as char array or std::string and will be
 * stored in the exception.
 * @code
 * throw (bda::generateThriftException<LightBIS::Exception>("Error xyz"));
 * @endcode
 */
template<typename T, typename U>
T generateThriftException(U&& aMessage) {
    T vException;
    vException.mMessage = aMessage;
    return vException;
}

/** @brief Create a thrift protocol factory with default settings. */
std::shared_ptr<apache::thrift::protocol::TProtocolFactory> createProtocolFactory(const bda::ProtocolType aProtocolType);

}

#endif
