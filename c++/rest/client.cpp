//g++ client.cpp -o client -I /mnt/d/sdk/cpprestsdk/Release/include -L /mnt/d/sdk/cpprestsdk/build.debug.static/Release/Binaries -lcpprest -lssl -lcrypto -lz


#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>

//using namespace web;
//using namespace web::http;
//using namespace web::http::client;

int main() {
	// Create an HTTP client to send the request.
	web::http::client::http_client client("http://localhost:8081");

	// ----------- GET Request -----------
	client.request(web::http::methods::GET).then([](web::http::http_response response) {
			if (response.status_code() == web::http::status_codes::OK) {
			return response.extract_json();
			}
			throw std::runtime_error("GET request failed");
			}).then([](web::json::value jsonResponse) {
				std::cout << "GET response: " << jsonResponse.serialize() << std::endl;
				}).wait();

	// ----------- POST Request -----------
	web::json::value postData;
	postData["name"] = web::json::value::string("Siva");
	postData["age"] = web::json::value::number(36);

	client.request(web::http::methods::POST, "/", postData).then([](web::http::http_response response) {
			if (response.status_code() == web::http::status_codes::OK) {
			return response.extract_json();
			}
			throw std::runtime_error("POST request failed");
			}).then([](web::json::value jsonResponse) {
				std::cout << "POST response: " << jsonResponse.serialize() << std::endl;
				}).wait();

	return 0;
}

