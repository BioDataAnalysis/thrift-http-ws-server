//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include "bda/ThriftHTTPWSServer.hh"

#include <bda/Helpers.hh>

#include <thrift/TProcessor.h>
#include <thrift/TToString.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <bda/bdanetworkservice_export.h>


namespace bda {

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type(boost::beast::string_view path) {
    using boost::beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == boost::beast::string_view::npos)
            return boost::beast::string_view{};
        return path.substr(pos);
    }();

    // clang-format off
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    // clang-format on

    return "application/text";
}

void ThriftHTTPWSServer::load_server_certificate(boost::asio::ssl::context& aSSLContext) {
    /**
     * This certificate was generated from CMD.EXE on Windows 10 using:
     *
     *   winpty openssl dhparam -out dh.pem 2048
     *   winpty openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem -subj "//C=US\ST=CA\L=Los Angeles\O=Beast\CN=www.example.com"
     */

    std::string const cert =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
        "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
        "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
        "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
        "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
        "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
        "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
        "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
        "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
        "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
        "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
        "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
        "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
        "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
        "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
        "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
        "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
        "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
        "UEVbkhd5qstF6qWK\n"
        "-----END CERTIFICATE-----\n";

    std::string const key =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
        "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
        "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
        "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
        "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
        "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
        "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
        "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
        "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
        "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
        "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
        "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
        "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
        "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
        "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
        "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
        "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
        "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
        "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
        "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
        "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
        "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
        "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
        "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
        "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
        "pVOUFq5mW8p0zbtfHbjkgxyF\n"
        "-----END PRIVATE KEY-----\n";

    std::string const dh =
        "-----BEGIN DH PARAMETERS-----\n"
        "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
        "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
        "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
        "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
        "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
        "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
        "-----END DH PARAMETERS-----\n";

    aSSLContext.set_password_callback(
        [](std::size_t,
           boost::asio::ssl::context_base::password_purpose) {
            return "test";
        });

    aSSLContext.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::single_dh_use);

    aSSLContext.use_certificate_chain(
        boost::asio::buffer(cert.data(), cert.size()));

    aSSLContext.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        boost::asio::ssl::context::file_format::pem);

    aSSLContext.use_tmp_dh(
        boost::asio::buffer(dh.data(), dh.size()));
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(const boost::beast::string_view base,
                     const boost::beast::string_view path) {
    if (base.empty()) {
        return std::string(path);
    }

    std::string result(base);

#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for (auto& c : result) {
        if (c == '/') {
            c = path_separator;
        }
    }
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator) {
        result.resize(result.size() - 1);
    }
    result.append(path.data(), path.size());
#endif

    return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<class Body, class Allocator, class Send>
void handle_request(boost::beast::string_view aHTTPDocumentRoot,
                    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& aHTTPRequest,
                    Send&& send) {
    // Returns a bad request response
    auto const bad_request =
        [&aHTTPRequest](boost::beast::string_view why) {
            boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::bad_request, aHTTPRequest.version() };
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(aHTTPRequest.keep_alive());
            res.body() = std::string(why);
            res.prepare_payload();
            return res;
        };

    // Returns a not found response
    auto const not_found =
        [&aHTTPRequest](boost::beast::string_view target) {
            boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::not_found, aHTTPRequest.version() };
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(aHTTPRequest.keep_alive());
            res.body() = "The resource '" + std::string(target) + "' was not found.";
            res.prepare_payload();
            return res;
        };

    // Returns a server error response
    auto const server_error =
        [&aHTTPRequest](boost::beast::string_view what) {
            boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::internal_server_error, aHTTPRequest.version() };
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, "text/html");
            res.keep_alive(aHTTPRequest.keep_alive());
            res.body() = "An error occurred: '" + std::string(what) + "'";
            res.prepare_payload();
            return res;
        };

    // Make sure we can handle the method
    if (aHTTPRequest.method() != boost::beast::http::verb::get &&
        aHTTPRequest.method() != boost::beast::http::verb::head) {
        BDAMessage(2, "handle_request(): Unknown HTTP-method '" + std::string(boost::beast::http::to_string(aHTTPRequest.method())) + "'.\n");
        return send(bad_request("Unknown HTTP-method"));
    }

    // Request path must be absolute and not contain "..".
    if (aHTTPRequest.target().empty() ||
        aHTTPRequest.target()[0] != '/' ||
        aHTTPRequest.target().find("..") != boost::beast::string_view::npos) {
        BDAMessage(2, "handle_request(): Illegal request-target '" + std::string(aHTTPRequest.target()) + "'.\n");
        return send(bad_request("Illegal request-target"));
    }

    // Build the path to the requested file
    std::string path = path_cat(aHTTPDocumentRoot, aHTTPRequest.target());
    if (aHTTPRequest.target().back() == '/') {
        path.append("index.html");
    }

    // Attempt to open the file
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == boost::beast::errc::no_such_file_or_directory) {
        BDAMessage(2, "handle_request(): The resource '" + path + "' was not found.\n");
        return send(not_found(aHTTPRequest.target()));
    }

    // Handle an unknown error
    if (ec) {
        BDAMessage(2, "handle_request(): An unknown server error occurred: '" + std::string(ec.message()) + "'.\n");
        return send(server_error(ec.message()));
    }

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (aHTTPRequest.method() == boost::beast::http::verb::head) {
        boost::beast::http::response<boost::beast::http::empty_body> res{ boost::beast::http::status::ok, aHTTPRequest.version() };
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(aHTTPRequest.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    boost::beast::http::response<boost::beast::http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(boost::beast::http::status::ok, aHTTPRequest.version())
    };

    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(aHTTPRequest.keep_alive());

    return send(std::move(res));
}

// Report a failure
void fail(const boost::beast::error_code ec, char const* what) {
    // boost::asio::ssl::error::stream_truncated, also known as an SSL
    // "short read", indicates the peer closed the connection without
    // performing the required closing handshake (for example, Google
    // does this to improve performance). Generally this can be a security
    // issue, but if your communication protocol is self-terminated (as
    // it is with both HTTP and WebSocket) then you may simply ignore
    // the lack of close_notify.
    //
    // When a short read would cut off the end of an HTTP message,
    // Beast returns the error boost::beast::boost::beast::http::error::partial_message.
    // Therefore, if we see a short read here, it has occurred after
    // the message has been completed, so it is safe to ignore it.

    if (ec == boost::asio::ssl::error::stream_truncated) {
        return;
    }

    std::cerr << what << ": " << ec.message() << "\n";
}

// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class thrift_websocket_session {
    boost::beast::flat_buffer buffer_;

    std::shared_ptr<apache::thrift::protocol::TProtocolFactory> mThriftProtocolFactory;
    std::shared_ptr<apache::thrift::TProcessor> mThriftProcessor;

    // Construct an in-memory-transport backed by real memory for storing the response.
    std::shared_ptr<apache::thrift::transport::TTransport> mOutputTransport;
    std::shared_ptr<apache::thrift::protocol::TProtocol> mOutputProtocol;

    // Access the derived class (this is the Curiously Recurring Template Pattern).
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }

    // Start the asynchronous operation
    template<class Body, class Allocator>
    void do_accept(const boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> aHTTPRequest) {
        BDAMessage(12, "thrift_websocket_session::do_accept(): Incoming websocket connection.\n");

        // Enable compression
        const uint8_t vCompression = 0;
        if (vCompression > 0) {
            boost::beast::websocket::permessage_deflate vDeflate;
            vDeflate.server_enable = true;
            vDeflate.compLevel = vCompression;
            derived().ws().set_option(vDeflate);
        }

        // Set the websocket to binary mode:
        derived().ws().text(false);

        // Disable automatic fragmentation:
        derived().ws().auto_fragment(false);

        // Set suggested timeout settings for the websocket
        derived().ws().set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        derived().ws().set_option(boost::beast::websocket::stream_base::decorator(
            [](boost::beast::websocket::response_type& res) {
                res.set(boost::beast::http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " advanced-server-flex");
            }));

        // Accept the websocket handshake
        derived().ws().async_accept(aHTTPRequest, boost::beast::bind_front_handler(&thrift_websocket_session::on_accept, derived().shared_from_this()));
    }

    void on_accept(const boost::beast::error_code ec) {
        BDAMessage(12, "thrift_websocket_session::on_accept(): Accepted websocket connection.\n");

        if (ec) {
            BDAMessage(2, "thrift_websocket_session::on_accept(): A server error occurred: '" + std::string(ec.message()) + "'.\n");
            return fail(ec, "accept");
        }

        // Read the next message
        do_read();
    }

    void do_read() {
        BDAMessage(12, "thrift_websocket_session::do_read(): Reading websocket message.\n");

        // Read a message into our buffer
        derived().ws().async_read(buffer_, boost::beast::bind_front_handler(&thrift_websocket_session::on_read, derived().shared_from_this()));
    }

    void on_read(const boost::beast::error_code ec, const std::size_t bytes_transferred) {
        BDAMessage(12, "thrift_websocket_session::on_read(): Received message of " + std::to_string(bytes_transferred) + " bytes.\n");

        // This indicates that the thrift_websocket_session was closed
        if (ec == boost::beast::websocket::error::closed) {
            BDAMessage(9, "thrift_websocket_session::on_read(): Connection closed.\n");
            return;
        }

        if (ec) {
            BDAMessage(2, "thrift_websocket_session::on_read(): Failed to read.\n");
            fail(ec, "read");
        }


        // Construct a temporary in-memory-transport as a shallow copy of the
        // input boost::beast::flat_buffer data, to avoid copying the data
        boost::beast::flat_buffer::mutable_data_type vBufferData = buffer_.data();
        std::shared_ptr<apache::thrift::transport::TTransport> vInputTransport;
        vInputTransport = std::make_shared<apache::thrift::transport::TMemoryBuffer>(reinterpret_cast<uint8_t*>(vBufferData.data()), vBufferData.size());
        std::shared_ptr<apache::thrift::protocol::TProtocol> vInputProtocol = mThriftProtocolFactory->getProtocol(vInputTransport);


        /** @todo fixme: Due to issue https://issues.apache.org/jira/browse/THRIFT-5108 we need to re-create the
         * Transport after every write operation. This is quite inefficient, but how to improve it? We can not
         * just "clear" the Tranport, and consume() does not free it.
         */
        mOutputTransport = std::make_shared<apache::thrift::transport::TMemoryBuffer>();
        mOutputProtocol = mThriftProtocolFactory->getProtocol(mOutputTransport);


        try {
            // Have the thrift processor process the message and respond to it
            void* vProcessorConnectionContext = nullptr;
            if (!mThriftProcessor->process(vInputProtocol, mOutputProtocol, vProcessorConnectionContext)) {
                return;
            }
        } catch (const apache::thrift::transport::TTransportException& ttx) {
            switch (ttx.getType()) {
                case apache::thrift::transport::TTransportException::END_OF_FILE:
                case apache::thrift::transport::TTransportException::INTERRUPTED:
                case apache::thrift::transport::TTransportException::TIMED_OUT:
                    // Client disconnected or was interrupted or did not respond within the receive timeout.
                    // No logging needed.  Done.
                    return;
                default: {
                    // All other transport exceptions are logged.
                    // State of connection is unknown.  Done.
                    std::cerr << "TConnectedClient died: " << ttx.what() << std::endl;
                    return;
                }
            }
        } catch (const apache::thrift::TException& tex) {
            std::cerr << "TConnectedClient processing exception: " << tex.what() << std::endl;
            return;
        }


        // Get the size and pointer to the output buffer. NOTE: We must ask
        // borrow() for at least one byte, in order for it to work correctly:
        uint32_t vThriftProcessorOutputSize = 1;
        const uint8_t* vThriftProcessorOutputPtr = mOutputTransport->borrow(nullptr, &vThriftProcessorOutputSize);
        BDAMessage(12, "thrift_websocket_session::on_read(): Generated answer of " + std::to_string(vThriftProcessorOutputSize) + " bytes.\n");

        ::boost::asio::const_buffer vOutputBufferWrapper(vThriftProcessorOutputPtr, vThriftProcessorOutputSize);
        derived().ws().async_write(vOutputBufferWrapper, boost::beast::bind_front_handler(&thrift_websocket_session::on_write, derived().shared_from_this()));
    }

    void on_write(const boost::beast::error_code ec, const std::size_t bytes_transferred) {
        BDAMessage(12, "thrift_websocket_session::on_write(): Sent a message of " + std::to_string(bytes_transferred) + " bytes.\n");

        if (ec) {
            BDAMessage(2, "thrift_websocket_session::on_write(): Failed to write.\n");
            return fail(ec, "write");
        }

        // Clear the input buffer:
        buffer_.consume(buffer_.size());

        // Clear the output buffer:
        mOutputProtocol.reset();
        mOutputTransport.reset();

        // Do another read
        do_read();
    }

public:
    // Start the asynchronous operation
    template<class Body, class Allocator>
    void run(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> aHTTPRequest,
             std::shared_ptr<apache::thrift::protocol::TProtocolFactory> aThriftProtocolFactory,
             std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor) {
        mThriftProtocolFactory = aThriftProtocolFactory;
        mThriftProcessor = aThriftProcessor;

        // Accept the WebSocket upgrade request
        do_accept(std::move(aHTTPRequest));
    }
};

// Handles a plain WebSocket connection
class plain_websocket_session
    : public thrift_websocket_session<plain_websocket_session>,
      public std::enable_shared_from_this<plain_websocket_session> {
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;

public:
    // Create the session
    explicit plain_websocket_session(boost::beast::tcp_stream&& stream)
        : ws_(std::move(stream)) {
    }

    // Called by the base class
    boost::beast::websocket::stream<boost::beast::tcp_stream>& ws() {
        return ws_;
    }
};

// Handles an SSL WebSocket connection
class ssl_websocket_session
    : public thrift_websocket_session<ssl_websocket_session>,
      public std::enable_shared_from_this<ssl_websocket_session> {
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;

public:
    // Create the ssl_websocket_session
    explicit ssl_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream)
        : ws_(std::move(stream)) {
    }

    // Called by the base class
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>& ws() {
        return ws_;
    }
};

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class http_session {
    // Access the derived class, this is part of
    // the Curiously Recurring Template Pattern idiom.
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }

    // This queue is used for HTTP pipelining.
    class queue {
        enum {
            // Maximum number of responses we will queue
            limit = 8
        };

        // The type-erased, saved work item
        struct work {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        http_session& self_;
        std::vector<std::unique_ptr<work>> items_;

    public:
        explicit queue(http_session& self)
            : self_(self) {
            static_assert(limit > 0, "queue limit must be positive");
            items_.reserve(limit);
        }

        // Returns true if we have reached the queue limit
        bool is_full() const {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns true if the caller should initiate a read
        bool on_write() {
            BOOST_ASSERT(!items_.empty());
            auto const was_full = is_full();
            items_.erase(items_.begin());
            if (!items_.empty()) {
                (*items_.front())();
            }
            return was_full;
        }

        // Called by the HTTP handler to send a response.
        template<bool isRequest, class Body, class Fields>
        void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) {
            // This holds a work item
            struct work_impl : work {
                http_session& self_;
                boost::beast::http::message<isRequest, Body, Fields> msg_;

                work_impl(http_session& self, boost::beast::http::message<isRequest, Body, Fields>&& msg)
                    : self_(self), msg_(std::move(msg)) {
                }

                void operator()() {
                    boost::beast::http::async_write(
                        self_.derived().stream(),
                        msg_,
                        boost::beast::bind_front_handler(
                            &http_session::on_write,
                            self_.derived().shared_from_this(),
                            msg_.need_eof()));
                }
            };

            // Allocate and store the work
            items_.push_back(boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if (items_.size() == 1) {
                (*items_.front())();
            }
        }
    };

    const std::string& mHTTPDocumentRoot;
    queue queue_;

    // The parser is stored in an optional container so we can
    // construct it from scratch at the beginning of each new message.
    boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

protected:
    boost::beast::flat_buffer buffer_;

    std::shared_ptr<apache::thrift::protocol::TProtocolFactory> mThriftProtocolFactory;
    std::shared_ptr<apache::thrift::TProcessor> mThriftProcessor;

    template<class Body, class Allocator>
    void make_websocket_session(boost::beast::tcp_stream stream,
                                boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> aHTTPRequest) {
        std::make_shared<plain_websocket_session>(std::move(stream))->run(std::move(aHTTPRequest), mThriftProtocolFactory, mThriftProcessor);
    }

    template<class Body, class Allocator>
    void make_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream,
                                boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> aHTTPRequest) {
        std::make_shared<ssl_websocket_session>(std::move(stream))->run(std::move(aHTTPRequest), mThriftProtocolFactory, mThriftProcessor);
    }

public:
    // Construct the session
    http_session(boost::beast::flat_buffer buffer, const std::string& aHTTPDocumentRoot,
                 std::shared_ptr<apache::thrift::protocol::TProtocolFactory> aThriftProtocolFactory,
                 std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor)
        : mHTTPDocumentRoot(aHTTPDocumentRoot), queue_(*this), buffer_(std::move(buffer)) {
        mThriftProtocolFactory = aThriftProtocolFactory;
        mThriftProcessor = aThriftProcessor;
    }

    void do_read() {
        // Construct a new parser for each message
        parser_.emplace();

        // Apply a reasonable limit to the allowed size
        // of the body in bytes to prevent abuse.
        parser_->body_limit(11 * 1024 * 1024);

        // Set the timeout.
        boost::beast::get_lowest_layer(derived().stream()).expires_after(std::chrono::seconds(300));

        // Read a request using the parser-oriented interface
        boost::beast::http::async_read(
            derived().stream(),
            buffer_,
            *parser_,
            boost::beast::bind_front_handler(
                &http_session::on_read,
                derived().shared_from_this()));
    }

    void on_read(const boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == boost::beast::http::error::end_of_stream) {
            return derived().do_eof();
        }

        if (ec) {
            return fail(ec, "read");
        }

        // See if it is a WebSocket Upgrade
        if (boost::beast::websocket::is_upgrade(parser_->get())) {
            // Disable the timeout.
            // The boost::beast::websocket::stream uses its own timeout settings.
            boost::beast::get_lowest_layer(derived().stream()).expires_never();

            // Create a websocket session, transferring ownership
            // of both the socket and the HTTP request.
            return make_websocket_session(derived().release_stream(), parser_->release());
        }

        // Send the response
        handle_request(mHTTPDocumentRoot, parser_->release(), queue_);

        // If we aren't at the queue limit, try to pipeline another request
        if (!queue_.is_full()) {
            do_read();
        }
    }

    void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            return fail(ec, "write");
        }

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return derived().do_eof();
        }

        // Inform the queue that a write completed
        if (queue_.on_write()) {
            // Read another request
            do_read();
        }
    }
};

// Handles a plain HTTP connection
class plain_http_session
    : public http_session<plain_http_session>,
      public std::enable_shared_from_this<plain_http_session> {
    boost::beast::tcp_stream stream_;

public:
    // Create the session
    plain_http_session(
        boost::beast::tcp_stream&& stream,
        boost::beast::flat_buffer&& buffer,
        const std::string& aHTTPDocumentRoot,
        std::shared_ptr<apache::thrift::protocol::TProtocolFactory> aThriftProtocolFactory,
        std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor)
        : http_session<plain_http_session>(std::move(buffer), aHTTPDocumentRoot, aThriftProtocolFactory, aThriftProcessor),
          stream_(std::move(stream)) {
    }

    // Start the session
    void run() {
        this->do_read();
    }

    // Called by the base class
    boost::beast::tcp_stream& stream() {
        return stream_;
    }

    // Called by the base class
    boost::beast::tcp_stream release_stream() {
        return std::move(stream_);
    }

    // Called by the base class
    void do_eof() {
        // Send a TCP shutdown
        boost::beast::error_code ec;
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

// Handles an SSL HTTP connection
class ssl_http_session
    : public http_session<ssl_http_session>,
      public std::enable_shared_from_this<ssl_http_session> {
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;

public:
    // Create the http_session
    ssl_http_session(
        boost::beast::tcp_stream&& stream,
        boost::asio::ssl::context& aSSLContext,
        boost::beast::flat_buffer&& buffer,
        const std::string& aHTTPDocumentRoot,
        std::shared_ptr<apache::thrift::protocol::TProtocolFactory> aThriftProtocolFactory,
        std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor)
        : http_session<ssl_http_session>(std::move(buffer), aHTTPDocumentRoot, aThriftProtocolFactory, aThriftProcessor),
          stream_(std::move(stream), aSSLContext) {
    }

    // Start the session
    void run() {
        // Set the timeout.
        boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(300));

        // Perform the SSL handshake
        // Note, this is the buffered version of the handshake.
        stream_.async_handshake(
            boost::asio::ssl::stream_base::server,
            buffer_.data(),
            boost::beast::bind_front_handler(
                &ssl_http_session::on_handshake,
                shared_from_this()));
    }

    // Called by the base class
    boost::beast::ssl_stream<boost::beast::tcp_stream>& stream() {
        return stream_;
    }

    // Called by the base class
    boost::beast::ssl_stream<boost::beast::tcp_stream> release_stream() {
        return std::move(stream_);
    }

    // Called by the base class
    void do_eof() {
        // Set the timeout.
        boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(300));

        // Perform the SSL shutdown
        stream_.async_shutdown(boost::beast::bind_front_handler(&ssl_http_session::on_shutdown, shared_from_this()));
    }

private:
    void on_handshake(const boost::beast::error_code ec, std::size_t bytes_used) {
        if (ec) {
            return fail(ec, "handshake");
        }

        // Consume the portion of the buffer used by the handshake
        buffer_.consume(bytes_used);

        do_read();
    }

    void on_shutdown(const boost::beast::error_code ec) {
        if (ec) {
            return fail(ec, "shutdown");
        }

        // At this point the connection is closed gracefully
    }
};

// Detects SSL handshakes
class detect_session : public std::enable_shared_from_this<detect_session> {
    boost::beast::tcp_stream stream_;
    boost::asio::ssl::context& mSSLContext;
    const std::string& mHTTPDocumentRoot;
    boost::beast::flat_buffer buffer_;

    std::shared_ptr<apache::thrift::protocol::TProtocolFactory> mThriftProtocolFactory;
    std::shared_ptr<apache::thrift::TProcessor> mThriftProcessor;

public:
    explicit detect_session(boost::asio::ip::tcp::socket&& socket,
                            boost::asio::ssl::context& aSSLContext,
                            const std::string& aHTTPDocumentRoot,
                            std::shared_ptr<apache::thrift::protocol::TProtocolFactory> aThriftProtocolFactory,
                            std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor)
        : stream_(std::move(socket)), mSSLContext(aSSLContext), mHTTPDocumentRoot(aHTTPDocumentRoot) {
        mThriftProtocolFactory = aThriftProtocolFactory;
        mThriftProcessor = aThriftProcessor;
    }

    // Launch the detector
    void run() {
        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(300));

        boost::beast::async_detect_ssl(stream_, buffer_, boost::beast::bind_front_handler(&detect_session::on_detect, this->shared_from_this()));
    }

    void on_detect(const boost::beast::error_code ec, bool result) {
        if (ec) {
            return fail(ec, "detect");
        }

        if (result) {
            // Launch SSL session
            std::make_shared<ssl_http_session>(std::move(stream_), mSSLContext, std::move(buffer_), mHTTPDocumentRoot, mThriftProtocolFactory, mThriftProcessor)->run();
        } else {
            // Launch plain session
            std::make_shared<plain_http_session>(std::move(stream_), std::move(buffer_), mHTTPDocumentRoot, mThriftProtocolFactory, mThriftProcessor)->run();
        }
    }
};

// Accepts incoming connections and launches the sessions
class HTTPConnectListener : public std::enable_shared_from_this<HTTPConnectListener> {
    std::shared_ptr<boost::asio::io_context> mIOContext;
    boost::asio::ssl::context& mSSLContext;
    boost::asio::ip::tcp::acceptor acceptor_;
    const std::string& mHTTPDocumentRoot;

    std::shared_ptr<apache::thrift::protocol::TProtocolFactory> mThriftProtocolFactory;
    std::shared_ptr<apache::thrift::TProcessor> mThriftProcessor;

public:
    HTTPConnectListener(std::shared_ptr<boost::asio::io_context> aIOContext,
                        boost::asio::ssl::context& aSSLContext,
                        boost::asio::ip::tcp::endpoint endpoint,
                        const std::string& aHTTPDocumentRoot,
                        std::shared_ptr<apache::thrift::protocol::TProtocolFactory> aThriftProtocolFactory,
                        std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor)
        : mIOContext(aIOContext), mSSLContext(aSSLContext), acceptor_(boost::asio::make_strand(*aIOContext)), mHTTPDocumentRoot(aHTTPDocumentRoot) {
        mThriftProtocolFactory = aThriftProtocolFactory;
        mThriftProcessor = aThriftProcessor;

        boost::beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run() {
        do_accept();
    }

private:
    void do_accept() {
        // The new connection gets its own strand
        acceptor_.async_accept(boost::asio::make_strand(*mIOContext), boost::beast::bind_front_handler(&HTTPConnectListener::on_accept, shared_from_this()));
    }

    void on_accept(const boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (ec) {
            fail(ec, "accept");
        } else {
            // Create the detector http_session and run it
            std::make_shared<detect_session>(std::move(socket), mSSLContext, mHTTPDocumentRoot, mThriftProtocolFactory, mThriftProcessor)->run();
        }

        // Accept another connection
        do_accept();
    }
};

ThriftHTTPWSServer::ThriftHTTPWSServer(const std::string& aServerURL, const unsigned short aPort,
                                       const std::string& aHTTPDocumentRoot, const int aThreads,
                                       std::shared_ptr<apache::thrift::TProcessor> aThriftProcessor,
                                       const bda::ProtocolType aProtocolType)
    : mThreads(aThreads) {
    // The SSL context is required to hold the SSL certificates
    boost::asio::ssl::context vSSLContext{ boost::asio::ssl::context::tlsv12 };

    // This holds the self-signed certificate used by the server
    load_server_certificate(vSSLContext);


    mIOContext = std::make_shared<boost::asio::io_context>(mThreads);

    // Create the thrift protocol for the transport. Note that we need to use
    // a thrift TMemoryBuffer for the transport because the actual send and
    // receive is done via boost::beast websockets. Note also that this code is
    // a bit simplified and currently only supports TBinaryProtocol, because
    // the constructor for TJSONProtocol has different arguments. This could
    // be fixed easily.
    std::shared_ptr<apache::thrift::protocol::TProtocolFactory> vThriftProtocolFactory = bda::createProtocolFactory(aProtocolType);

    boost::asio::ip::tcp::endpoint vServerEndpoint{ boost::asio::ip::make_address(aServerURL.c_str()), aPort };

    // Create and launch a listening port
    mConnectionListener = std::make_shared<bda::HTTPConnectListener>(mIOContext, vSSLContext, vServerEndpoint, aHTTPDocumentRoot,
                                                                     vThriftProtocolFactory, aThriftProcessor);
}

void ThriftHTTPWSServer::asyncRun() {
    mMainServerThread = std::make_shared<std::thread>(&bda::ThriftHTTPWSServer::backgroundRun, this);
}

void ThriftHTTPWSServer::backgroundRun() {
    mConnectionListener->run();

    // Run the I/O service on the requested number of threads
    mWebServerThreads.reserve(mThreads - 1);
    for (int i = mThreads - 1; i > 0; --i) {
        mWebServerThreads.emplace_back([&] {
            mIOContext->run();
        });
    }

    mIOContext->run();
    BDAMessage(8, "ThriftHTTPWSServer::backgroundRun(): The io_context stopped, so the server should end. Will join server threads.\n");

    // Block until all the threads exit
    for (auto& vWebServerThread : mWebServerThreads) {
        vWebServerThread.join();
    }

    BDAMessage(8, "ThriftHTTPWSServer::backgroundRun(): Joined server threads. The backgroundRun() method ended cleanly.\n");
}

void ThriftHTTPWSServer::stop() {
    BDAMessage(8, "ThriftHTTPWSServer::stop(): Stopping io-context\n");
    mIOContext->stop();

    BDAMessage(8, "ThriftHTTPWSServer::stop(): Joining main server thread\n");
    mMainServerThread->join();
}

}