// sudo apt install libcpprest-dev
// g++ -std=c++17 cpprest_client.cpp -o client -lcpprest -lssl -lcrypto -pthread -lz
//
// Start cpprest_server first, it listens on /api


#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>
#include <stdexcept>
#include <string>

// Returns the json of a successful response, and throws for any other status
static pplx::task<web::json::value> json_or_throw(const web::http::http_response& response, const std::string& name) {
	if (response.status_code() == web::http::status_codes::OK) {
		return response.extract_json();
	}
	throw std::runtime_error(name + " request failed with status " + std::to_string(response.status_code()));
}

int main() {
	// Create an HTTP client to send the request.
	web::http::client::http_client client("http://localhost:8081/api");

	// wait() rethrows any failure from the chain, such as the server being down, or a status other than OK
	try {
		// ----------- GET Request -----------
		client.request(web::http::methods::GET).then([](web::http::http_response response) {
			return json_or_throw(response, "GET");
		}).then([](web::json::value jsonResponse) {
			std::cout << "GET response: " << jsonResponse.serialize() << std::endl;
		}).wait();

		// ----------- POST Request -----------
		web::json::value postData;
		postData["name"] = web::json::value::string("Siva");
		postData["age"] = web::json::value::number(36);

		client.request(web::http::methods::POST, "", postData).then([](web::http::http_response response) {
			return json_or_throw(response, "POST");
		}).then([](web::json::value jsonResponse) {
			std::cout << "POST response: " << jsonResponse.serialize() << std::endl;
		}).wait();
	} catch (const std::exception& e) {
		std::cerr << "Request failed: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
