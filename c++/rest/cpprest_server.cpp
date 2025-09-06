// g++ cpprest_server.cpp -o server -I /mnt/d/sdk/cpprestsdk/Release/include -L /mnt/d/sdk/cpprestsdk/build.debug.static/Release/Binaries -lcpprest -lssl -lcrypto -lpthread -lz


#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <optional>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

//using namespace web;
//using namespace web::http;
//using namespace web::http::experimental::listener;

std::vector<std::string> stringsplit(const std::string str, char delimeter = ',')
{
	std::vector<std::string> vec;
	std::istringstream iss(str);
	std::string token;

	while ( std::getline(iss, token, delimeter) )
	{
		vec.push_back(token);
	}
	return vec;
}

struct User {
		int id;
		std::string name;
		std::string email;

		friend std::ostream& operator<< (std::ostream& os, const User& user)
		{
			os << "id = " << user.id << ", name = " << user.name << ", email = " << user.email << std::endl;
			return os;
		}
};



void handle_get(web::http::http_request request) {

    // Request Headers
    for ( auto &header : request.headers() )
    {
	    std::cout << "header <" << header.first << "> <" << header.second << ">" << std::endl;
    }

    // Request URL
    std::cout << "request url          : " << request.request_uri().to_string() << std::endl;
    std::cout << "request absolute url : " << request.absolute_uri().to_string() << std::endl;

    // Request Param
    std::cout << "request param        : " << request.request_uri().path() << std::endl; 

    web::json::value response_data;
    response_data["message"] = web::json::value::string("Hello from C++ REST SDK Server!");

    request.reply(web::http::status_codes::OK, response_data);
}

void handle_put(web::http::http_request request) {
    std::cout << "PUT request received" << std::endl;

    web::json::value response_data;
    response_data["message"] = web::json::value::string("Yet to implement PUT");

    request.reply(web::http::status_codes::OK, response_data);
}

void handle_post(web::http::http_request request) {
    std::cout << "POST request received" << std::endl;

    request.extract_json().then([request](pplx::task<web::json::value> task) {
        try {
	    web::json::value post_data = task.get();
            std::cout << "Received data: " << post_data.serialize() << std::endl;

	    web::json::value response;
            response["status"] = web::json::value::string("received");
            response["your_data"] = post_data;

            request.reply(web::http::status_codes::OK, response);
        } catch (const std::exception& e) {
            request.reply(web::http::status_codes::BadRequest, "Invalid JSON");
        }
    });
}

void handle_del(web::http::http_request request) {
    std::cout << "DEL request received" << std::endl;

    web::json::value response_data;
    response_data["message"] = web::json::value::string("Yet to implement DEL");

    request.reply(web::http::status_codes::OK, response_data);
}

void handle_patch(web::http::http_request request) {
    std::cout << "PATCH request received" << std::endl;

    web::json::value response_data;
    response_data["message"] = web::json::value::string("Yet to implement PATCH");

    request.reply(web::http::status_codes::OK, response_data);
}


int main() {
    web::http::experimental::listener::http_listener listener("http://localhost:8081/api");

    listener.support(web::http::methods::GET, handle_get);
    listener.support(web::http::methods::PUT, handle_put);
    listener.support(web::http::methods::POST, handle_post);
    listener.support(web::http::methods::DEL, handle_del);
    listener.support(web::http::methods::PATCH, handle_patch);

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
    }

    return 0;
}

