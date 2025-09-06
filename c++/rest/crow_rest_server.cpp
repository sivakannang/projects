/*

g++ crow_rest_server.cpp -o crow_server -I/mnt/d/sdk/Crow/include -I/mnt/d/sdk/json/include -lpthread -lmysqlcppconn

| Method               | Use for 
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------|
| `executeQuery(...)`  | Queries that return rows** (e.g., `SELECT ...`) , returns  `sql::ResultSet*`                                                     |
|                      | Throws if the SQL doesn’t produce a result set. Iterate with `rs->next()`.                                                       |
|                      |                                                                                                                                  |
| `executeUpdate(...)` | DML/DDL that don’t return rows (`INSERT`, `UPDATE`, `DELETE`, `CREATE TABLE`…) , returns `int` (rows affected)                   |
|                      | For DDL the return is typically 0                                                                                                |
|                      |                                                                                                                                  |
| `execute(...)`       | When you don’t know if the SQL returns rows (e.g., stored procedures that may return rows or just an update count), returns bool |
|                      | `true` ⇒ a result set is available: call `std::unique_ptr<std::ResultSet(statement->getResultSet())`
|                      |  use 'bool statement->getMoreResults()' to chk next set is available or not.
|                      | `false` ⇒ call `statement->getUpdateCount()`.                
-----------------------------------------------------------------------------------------------------------------------------------------------------------

Autocommit is ON by default → every statement is committed immediately.

To make a transaction: turn autocommit OFF, run multiple statements, then commit() or rollback().


bool transferFunds(sql::Connection& conn, int from, int to, int amount) {
    try {
        conn.setAutoCommit(false); // begin
        ....
        conn.commit();              // success
        conn.setAutoCommit(true);   // restore
    } catch (...) {
        try { conn.rollback(); } catch (...) {}
        conn.setAutoCommit(true);
    }
}

RAII Style:

struct Tx {
    sql::Connection& c;
    bool active{true};
    explicit Tx(sql::Connection& c_) : c(c_) { c.setAutoCommit(false); }
    void commit() { if (active) { c.commit(); c.setAutoCommit(true); active = false; } }
    ~Tx() { if (active) { try { c.rollback(); } catch (...) {} try { c.setAutoCommit(true); } catch(...){} } }
};

// usage
bool saveUser(sql::Connection& conn, const User& u) {
    Tx tx(conn);
    std::unique_ptr<sql::PreparedStatement> ps(
        conn.prepareStatement("INSERT INTO users(name,email) VALUES(?,?)"));
    ps->setString(1, u.name);
    ps->setString(2, u.email);
    if (ps->executeUpdate() != 1) return false;
    tx.commit();
    return true;
}



Data Types -> setInt, setUInt, setInt64, setUInt64, setDouble, setBoolean, setString, setDateTime, setBlop, setNull

use std::unique_ptr to store sql::PreparedStaement, sql::Statement, sql::ResultSet for avoid memory leak, else we need to delete manually

-----------------------------------------------------------------------------------------------------------------------------------------------------------
JSON → Object ( nested example )
    std::string raw = R"({
        "id": 2,
        "name": "Kannan",
        "address": {
            "city": "Coimbatore",
            "pincode": "641001"
        }
    })";

JSON → Object
    std::string raw = R"({
        "id": 2,
        "name": "Kannan",
        "email": "kannan@google.com"
    })";

    User u = json::parse(raw);



    // Enforce all fields mandatory, use std::optional to make it optional
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, id, name, email)

    // Fields are optional, json parsing won't throw if json keys not present
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(User, id, name, email)


    nlohmann json parsing prototype:
    	template <typename InputType>
	static nlohmann::json parse(InputType&& i,
	nlohmann::json::parser_callback_t cb = nullptr,
	bool allow_exceptions = true,
	bool ignore_comments = false,
	bool ignore_trailing_commas = false);

	auto j = json::parse(s,
			nullptr,	// callback
			false,		// allow_exceptions
                        true,           // ignore_comments
			true);		// ignore trailing commas
    if (j.is_discarded()) return false; // check json is valid or not

-------------------------------------------------------------------------------------------------------------------------------
*/


#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <optional>

#include <crow.h>

#include <nlohmann/json.hpp>

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

using json = nlohmann::json;


inline sql::Driver* mysql_driver() { static auto* d = get_driver_instance(); return d; }

inline std::unique_ptr<sql::Connection> make_conn() {
    auto* c = mysql_driver()->connect("tcp://127.0.0.1:3306", "siva", "kannan");
    c->setSchema("testdb");
    return std::unique_ptr<sql::Connection>(c);
}

static thread_local std::unique_ptr<sql::Connection> tls_conn;

inline sql::Connection& conn() {
    if (!tls_conn) tls_conn = make_conn();
    return *tls_conn;
}

class User {
	public:
		int id;
		std::string name;
		std::string email;

		User(int id = 0, std::string name = "", std::string email = "") : id(id), name(name), email(email) {}

		friend std::ostream& operator << (std::ostream& os, const User& user) {
			os << user.id << " , " << user.name << " , " << user.email ;
			return os;
		}

		//NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, id, name, email)
		NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(User, id, name, email)
};

class UserRepository {
	public:
		virtual ~UserRepository() = default;
		virtual std::optional<User> findById(int id) = 0;
		virtual std::vector<User> findAll() = 0;
		virtual bool save(const User& user) = 0;
		virtual bool deleteById(int id) = 0;
};

class UserRepositoryImpl : public UserRepository {
	
	public:
		
		std::optional<User> findById(int id) override
		{
			std::unique_ptr<sql::PreparedStatement> stmt ( conn().prepareStatement("SELECT * FROM users WHERE id=?"));
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

		std::vector<User> findAll() override
		{
			std::vector<User> users;
			std::unique_ptr<sql::Statement> stmt ( conn().createStatement());
			std::unique_ptr<sql::ResultSet> res (stmt->executeQuery("SELECT * FROM users"));
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
			std::unique_ptr<sql::PreparedStatement> stmt ( conn().prepareStatement("DELETE FROM users WHERE id=?") );
			stmt->setInt(1, id);
			return stmt->executeUpdate() == 1 ;
		}

		bool save(const User& user) override
		{
			if ( user.id == 0 )
			{
				std::unique_ptr<sql::PreparedStatement> stmt ( conn().prepareStatement("INSERT INTO users (name, email) VALUES (?, ?)"));
				stmt->setString(1, user.name);
				stmt->setString(2, user.email);
				return stmt->executeUpdate() == 1 ;
			}
			else
			{
				std::unique_ptr<sql::PreparedStatement> stmt ( conn().prepareStatement("UPDATE users SET name=?, email=? WHERE id=?"));
				stmt->setString(1, user.name);
				stmt->setString(2, user.email);
				stmt->setInt(3, user.id);
				return stmt->executeUpdate() == 1;
			}
		}
};

void crow_rest_api()
{
	crow::SimpleApp app;
	UserRepositoryImpl userRepo;

	CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::GET)
		( [&userRepo] () {
		  	std::vector<User> users = userRepo.findAll();
		  	json j = users;
		  	crow::response res(200, j.dump(4));
		  	res.set_header("Content-Type", "application/json");
		  	return res;
		  } );

	CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::GET)
		( [&userRepo] (int id) {
		  	std::optional<User> user = userRepo.findById(id);
		  	if ( !user )
		  		return crow::response(404, "user not found");
		  	json j = *user;
		  	crow::response res(200, j.dump(4));
		  	res.set_header("Content-Type", "application/json");
		  	return res;
		  } );

	CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::DELETE)
		( [&userRepo] (int id) {
		  	bool ok = userRepo.deleteById(id);
		  	return ok ? crow::response(204) : crow::response{404, "user not found"};
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

			if ( req.body.size() > 1 * 1024 * 1024 ) {
				return crow::response{413, "payload too large"};
			}

		  	//json j = json::parse(req.body);
		  	json j = json::parse(req.body, nullptr, false, true, true); // set allow exception false, so it won't throw
			if ( j.is_discarded() || !j.is_object() )
				return crow::response{400, "invalid json"};
			User user = j.get<User>();
			bool ok = userRepo.save(user);
			if (!ok) return crow::response{500, "db error"};

			crow::response res(201, json(user).dump(4));
			res.set_header("Content-Type", "application/json");
			return res;

		  } );

	std::cout << "std::thread::hardware_concurrency() : " << std::thread::hardware_concurrency() << std::endl;

	//app.port(8082).concurrency(12).run();                    // blocking call, allows to specify number of threads
	//app.port(8081).multithreaded().run();                    // blocking call, use std::hardware_concurrency to determine number of threads
	//auto fut = app.port(8081).multithreaded().run_async();   // non blocking call, runs in async, do other work, finally call app.stop() and fut.get()

	//std::thread web ( [&app] () {
	//		app.bindaddr("0.0.0.0").port(8081).multithreaded().run();
	//		});
	//std::this_thread::sleep_for(std::chrono::hours(1));
	//app.stop();
	//web.join();
	
	app.port(8081).multithreaded().run();
}

int main()
{
	crow_rest_api();
	return 0;
}
