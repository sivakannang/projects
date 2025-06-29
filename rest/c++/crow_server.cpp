/*
 g++ crow_server.cpp -o crow_server -I/mnt/d/sdk/Crow/include -I/mnt/d/sdk/json/include -lboost_system -lpthread -lmysqlclient 
 g++ crow_server.cpp -o crow_server -I/mnt/d/sdk/Crow/include -I/mnt/d/sdk/json/include -lpthread -lmysqlclient 
 *
 */

#include <iostream>
#include <string>
#include <optional>
#include <list>

#include "crow.h"
#include "nlohmann/json.hpp"
#include <mysql/mysql.h>


using json = nlohmann::json;

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


// nlohmann expects void to_json(json&, const T&) and void from_json(const json&, User&) signature to support below auto serialization/deserialization
// nlohmann::json library uses template specialization and ADL to enable this
// json j = obj;        // ✨ magic conversion
// obj = j.get<T>();    // ✨ reverse magic

void to_json(json& j, const User& user)
{
	j = { {"id", user.id}, {"name", user.name}, {"email", user.email} };
}

void from_json(const json& j, User& u)
{
	if ( j.contains("id"))
		j.at("id").get_to(u.id);
	u.name = j.value("name", "");  // if name is not present in json fallback to default value given
	u.email = j.value("email", "");
}

/***
 * JSON → Object ( nested example )
    std::string raw = R"({
        "id": 2,
        "name": "Kannan",
        "address": {
            "city": "Coimbatore",
            "pincode": "641001"
        }
    })";

 * JSON → Object
    std::string raw = R"({
        "id": 2,
        "name": "Kannan",
        "email": "kannan@google.com"
    })";

    User u = json::parse(raw);
*/

class SQLConn {
	private:
		MYSQL *conn;
		MYSQL_STMT *stmt;
		MYSQL_RES *result;
	public:
		SQLConn(const char* host, const char* user, const char* password, const char* db) : conn(NULL), stmt(NULL), result(NULL)
		{
			conn = mysql_init(NULL);
			if ( !conn ) throw std::runtime_error("mysql_init() failed");

			if (!mysql_real_connect(conn, host, user, password, db, 0, NULL, 0)) {
				throw std::runtime_error(mysql_error(conn));
			}
		}
		~SQLConn()
		{
			//if (stmt) mysql_stmt_close(stmt);
			if (conn) mysql_close(conn);
		}

		bool Query(std::string query)
		{
			if ( mysql_query(conn, query.c_str()) )
			{
				std::cout << "mysql_query() failed => " << query << " >> " << mysql_error(conn) << std::endl;
				return false;
			}
			return true;
		}

		std::optional<User> findById(int id)
		{
			std::optional<User> user = std::nullopt;

			std::string query = "SELECT *FROM users WHERE id=" + std::to_string(id);
			if ( Query(query) == false )
				return user;

			result = mysql_store_result(conn);
			if ( !result )
				return user;

			// print field names
			/*int num_fields = mysql_num_fields(result);
			MYSQL_FIELD *fields = mysql_fetch_fields(result);
			for (int i = 0; i < num_fields; i++) {
				printf("%s\t", fields[i].name);
			}*/

			// print field values
			MYSQL_ROW row;
			while ( row = mysql_fetch_row(result) )
			{
				user = User { std::stoi(row[0]), std::string(row[1]), std::string(row[2]) };
			}
			mysql_free_result(result);
			return user;
		}


		std::list<User> findAll()
		{
			std::list<User> users;

			std::string query = "SELECT *FROM users";
			if ( Query(query) == false )
				return users;

			result = mysql_store_result(conn);
			if ( !result )
				return users;

			// print field values
			MYSQL_ROW row;
			while ( row = mysql_fetch_row(result) )
			{
				users.push_back( User { std::stoi(row[0]), std::string(row[1]), std::string(row[2]) } );
			}
			mysql_free_result(result);
			return users;
		}

		int insert(const User& u)
		{
			/*** Prepared statement - avoid sql injection ( safety )
			stmt = mysql_stmt_init(conn);
			const char* sql = "INSERT INTO users (id, name, email) VALUES (?, ?, ?)";
			mysql_stmt_prepare(stmt, sql, strlen(sql));

			// Bind parameters safely
			MYSQL_BIND bind[3] = {0};
			bind[0].buffer_type = MYSQL_TYPE_LONG;
			bind[0].buffer = (void *)&u.id;

			bind[1].buffer_type = MYSQL_TYPE_STRING;
			bind[1].buffer = (void*)u.name.c_str();
			bind[1].buffer_length = u.name.length();

			bind[2].buffer_type = MYSQL_TYPE_STRING;
			bind[2].buffer = (void*)u.email.c_str();
			bind[2].buffer_length = u.email.length();

			mysql_stmt_bind_param(stmt, bind);
			mysql_stmt_execute(stmt);
			mysql_stmt_close(stmt);
			***/

			//mysql_insert_id(conn); // get the last insert id
			
			char query[512] = {0};
			snprintf(query, sizeof(query), "INSERT INTO users (name, email) VALUES ('%s', '%s')", u.name.c_str(), u.email.c_str());
			return ( Query(std::string(query)) && (mysql_affected_rows(conn) == 1) ) ? mysql_insert_id(conn) : -1; 
		}


		bool updateById(const User& u)
		{
			char query[512] = {0};
			snprintf(query, sizeof(query), "UPDATE users SET name = '%s', email = '%s' WHERE id = %d", u.name.c_str(), u.email.c_str(), u.id);
			bool status = Query(std::string(query) );
			if ( !status )
				return false;
			return ( mysql_affected_rows(conn) == 1 );
		}

		bool updateNameById(const User& u)
		{
			char query[512] = {0};
			snprintf(query, sizeof(query), "UPDATE users SET email = '%s' WHERE id = %d", u.email.c_str(), u.id);
			bool status = Query(std::string(query) );
			if ( !status )
				return false;
			return ( mysql_affected_rows(conn) == 1 );
		}
		bool deleteById(int id)
		{
			char query[512] = {0};
			snprintf(query, sizeof(query), "DELETE FROM users WHERE id = %d", id);
			bool status = Query(std::string(query) );
			if ( !status )
				return false;
			return ( mysql_affected_rows(conn) == 1 );
		}

};


int main() {

	crow::SimpleApp app;

	SQLConn sqlConn("localhost", "siva", "kannan", "testdb");

	// GET - /api/user { all users }
	CROW_ROUTE(app, "/api/user")
		([&sqlConn](){
		 std::list<User> users = sqlConn.findAll();
		 if ( users.empty() )
		 	return crow::response("users not found");
		 json j = users;
		 crow::response res( j.dump(4) );   // 4 spaces for indent level
		 res.set_header("Content-Type", "application/json");
		 return res;
		 });

	// GET /api/user/<id> { user by id }
	CROW_ROUTE(app, "/api/user/<int>")
		([&sqlConn](int id){
		 std::optional<User> user = sqlConn.findById(id);
		 if ( !user )
		 	return crow::response(200, "user id not found - " + std::to_string(id) );
		 //to_json(j, *user);
		 json j = *user;
		 return crow::response( j.dump() );
		 });

	// POST /api/user  { insert user }
	CROW_ROUTE(app, "/api/user").methods(crow::HTTPMethod::POST)
		([&sqlConn](const crow::request& req){
		 try
		 {
		 	json j = json::parse(req.body);
			User u = j.get<User>();
			u.id = sqlConn.insert(u);
		 	return crow::response( json(u).dump() );
		 }
		 catch(std::exception& e)
		 {
		 	return crow::response ( e.what() );
		 }
		 });

	// PUT /api/user/<id>  { update whole user by id }
	CROW_ROUTE(app, "/api/user/<int>").methods(crow::HTTPMethod::PUT)
		([&sqlConn](const crow::request& req, int id){
		 try
		 {
		 	json j = json::parse(req.body);
			User u = j.get<User>();
			u.id = id;
			if ( !sqlConn.updateById(u) )
				return crow::response( json::parse( R"({"id" : -1})" ).dump()  );
		 	return crow::response( json(u).dump() );
		 }
		 catch(std::exception& e)
		 {
		 	return crow::response ( e.what() );
		 }
		 });

	// PATCH /api/user/<id> { partial update by id }
	CROW_ROUTE(app, "/api/user/<int>").methods(crow::HTTPMethod::PATCH)
		([&sqlConn](const crow::request& req, int id){
		 try
		 {
		 	json j = json::parse(req.body);
			User u = j.get<User>();
			u.id = id;
			if ( !sqlConn.updateNameById(u) )
				return crow::response( json::parse( R"({"id" : -1})" ).dump()  );
		 	return crow::response( json(u).dump(4) );
		 }
		 catch(std::exception& e)
		 {
		 	return crow::response ( e.what() );
		 }
		 });

	// DELETE /api/user/<id> { delete by user id }
	CROW_ROUTE(app, "/api/user/<int>").methods(crow::HTTPMethod::DELETE)
		([&sqlConn](int id){
		 return crow::response( sqlConn.deleteById(id) ? "delete id success" : "delete id failed");
		 });

	app.port(8081).multithreaded().run();
}

