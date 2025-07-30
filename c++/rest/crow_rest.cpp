
// g++ crow_rest.cpp -o crow_server -I/mnt/d/sdk/Crow/include -I/mnt/d/sdk/json/include -lpthread -lmysqlcpplient

#include <iostream>
#include <string>
#include <list>
#include <crow.h>
#include <nlohmann/json.hpp>

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <memory>

using json = nlohmann::json;

class MySQLHelper {
	
	public:
		std::shared_ptr<sql::Connection> conn;
		
		MySQLHelper() {
			sql::Driver* driver = get_driver_instance();
			conn = std::shared_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", "siva", "kannan"));
			conn->setSchema("testdb");
		}
};

class User {
	public:
		int id;
		std::string name;
		std::string email;

		User(int id = 0, std::string name = "", std::string email = "") : id(id), name(name), email(email) {}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, id, name, email)
};

class UserRepository {
	public:
		virtual ~UserRepository() = default;
		virtual std::optional<User> findById(int id) = 0;
		virtual std::list<User> findAll() = 0;
		virtual bool save(const User& user) = 0;
		virtual bool deleteById(int id) = 0;
};

class UserRepositoryImpl : public UserRepository {
	private:
		MySQLHelper &db;
	public:
		UserRepositoryImpl(MySQLHelper &database) : db(database) {}
		
		std::optional<User> findById(int id) override
		{
			auto stmt = db.conn->prepareStatement("SELECT *FROM users WHERE id=?");
			stmt->setInt(1, id);

			std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
			if ( res->next() ) {
				return User(
						res->getInt("id"),
						res->getString("name"),
						res->getString("email")
				);
			}
			return std::nullopt;
		}

		std::list<User> findAll() override
		{
			std::list<User> users;
			auto stmt = db.conn->createStatement();
			std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT *FROM users"));
			while ( res->next() ) {
				users.emplace_back(
						res->getInt("id"),
						res->getString("name"),
						res->getString("email")
						);
			}
			return users;
		}

		bool deleteById(int id) override
		{
			auto stmt = db.conn->prepareStatement("DELETE FROM users WHERE id=?");
			stmt->setInt(1, id);
			stmt->execute();
			return true;
		}

		bool save(const User& user) override
		{
			if ( user.id == 0 )
			{
				auto stmt = db.conn->prepareStatement("INSERT INTO users (name, email) VALUES (?, ?)");
				stmt->setString(1, user.name);
				stmt->setString(2, user.email);
				stmt->execute();
			}
			else
			{
				auto stmt = db.conn->prepareStatement("UPDATE users SET name=?, email=? WHERE id=?");
				stmt->setString(1, user.name);
				stmt->setString(2, user.email);
				stmt->setInt(3, user.id);
				stmt->execute();
			}
			return true;
		}
};

/*
void to_json(json& j, const User& user)
{
	j = { {"id", user.id}, {"name", user.name}, {"email", user.email} };
}

void from_json(const json& j, User& user)
{
	if ( j.contains("id") )
		j.at("id").get_to(user.id);
	user.name = j.value("name", "");
	user.email = j.value("email", "");
}
*/


void crow_rest_api()
{
	crow::SimpleApp app;
	MySQLHelper db;
	UserRepositoryImpl userRepo(db);

	CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::GET)
		( [&userRepo] () {
		  	std::list<User> users = userRepo.findAll();
		  	json j = users;
			if ( users.empty() )
				return crow::response("no user records found");
		  	crow::response res(j.dump(4));
		  	res.set_header("Content-Type", "application/json");
		  	return res;
		  } );

	CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::GET)
		( [&userRepo] (int id) {
		  	std::optional<User> user = userRepo.findById(id);
		  	if ( !user )
		  		return crow::response(404, "user not found");
		  	json j = *user;
		  	return crow::response(200, j.dump(4));
		  } );

	CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::DELETE)
		( [&userRepo] (int id) {
		  	userRepo.deleteById(id);
		  	return crow::response("deleted");
		  } );

	CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::POST)
		( [&userRepo] (const crow::request &req) {

		  	auto val = req.get_header_value("content-type");
			if ( !val.empty() ) { std::cout << "header content-type : " << val << std::endl; }

		  	for ( const auto& header : req.headers )
				std::cout << header.first << " : " << header.second << std::endl;

			int page = 0, size = 0;		  
		  	auto query_params = crow::query_string(req.url_params);
	  		if (query_params.get("page")) {
        			page = std::stoi(query_params.get("page"));
    			}
    			if (query_params.get("size")) {
        			size = std::stoi(query_params.get("size"));
			}

		  	json j = json::parse(req.body);
			User user = j.get<User>();
			userRepo.save(user);
			return crow::response(201, "user insert/update ok");

		  } );

	app.port(8081).multithreaded().run();
}

int main()
{
	crow_rest_api();
	return 0;
}
