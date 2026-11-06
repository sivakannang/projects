// sudo apt install libcpprest-dev
// g++ -std=c++17 cpprest_server.cpp -o server -lcpprest -lssl -lcrypto -pthread -lz
//
// Note : Microsoft's cpprestsdk repository is archived on GitHub, so it gets no more updates. Prefer another library for new code.
//
// GET    /api?x=1   200, prints the request details
// POST   /api       200 echoes the json, 400 for invalid json, 415 if Content-Type isn't application/json
// PUT, DELETE, PATCH    501, not implemented yet


#include <iostream>
#include <string>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

void handle_get(web::http::http_request request) {

    // Request Headers. Don't do this in real code, Authorization and Cookie hold secrets.
    for ( auto &header : request.headers() )
    {
        std::cout << "header <" << header.first << "> <" << header.second << ">" << std::endl;
    }

    // Request URL
    std::cout << "request url          : " << request.request_uri().to_string() << std::endl;
    std::cout << "request absolute url : " << request.absolute_uri().to_string() << std::endl;
    std::cout << "request path         : " << request.request_uri().path() << std::endl;

    // Request Params
    for ( const auto &param : web::uri::split_query(request.request_uri().query()) )
    {
        std::cout << "request param        : " << param.first << " = " << param.second << std::endl;
    }

    web::json::value response_data;
    response_data["message"] = web::json::value::string("Hello from C++ REST SDK Server!");

    request.reply(web::http::status_codes::OK, response_data);
}

void handle_post(web::http::http_request request) {
    std::cout << "POST request received" << std::endl;

    // extract_json returns null without an error when there is no Content-Type, and the data is lost. So check it first.
    if ( request.headers().content_type().rfind("application/json", 0) != 0 ) {
        request.reply(web::http::status_codes::UnsupportedMediaType, "Content-Type must be application/json");
        return;
    }

    request.extract_json().then([request](pplx::task<web::json::value> task) {
        try {
            web::json::value post_data = task.get();
            std::cout << "Received data: " << post_data.serialize() << std::endl;

            web::json::value response;
            response["status"] = web::json::value::string("received");
            response["your_data"] = post_data;

            request.reply(web::http::status_codes::OK, response);
        } catch (const std::exception&) {
            request.reply(web::http::status_codes::BadRequest, "Invalid JSON");
        }
    });
}

void handle_not_implemented(web::http::http_request request) {
    std::cout << request.method() << " request received, not implemented" << std::endl;
    request.reply(web::http::status_codes::NotImplemented, "Not implemented");
}


int main() {
    web::http::experimental::listener::http_listener listener("http://localhost:8081/api");

    listener.support(web::http::methods::GET, handle_get);
    listener.support(web::http::methods::POST, handle_post);
    listener.support(web::http::methods::PUT, handle_not_implemented);
    listener.support(web::http::methods::DEL, handle_not_implemented);
    listener.support(web::http::methods::PATCH, handle_not_implemented);

    try {
        listener
            .open()
            .then([]() { std::cout << "Server is running on port 8081...\n"; })
            .wait();

        std::cout << "Press ENTER to exit.\n";
        std::string line;
        std::getline(std::cin, line);

        listener.close().wait();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
