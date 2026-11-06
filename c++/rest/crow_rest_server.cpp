/*

sudo apt install libasio-dev nlohmann-json3-dev libmysqlcppconn-dev
Crow is a single header. Get crow_all.h from its GitHub release, and save it as crow.h in a folder given with -I.

g++ -std=c++17 crow_rest_server.cpp -o crow_server -I/path/to/crow -pthread -lmysqlcppconn

export DB_USER=... DB_PASSWORD=...      Credentials stay out of the code. Optional : DB_URL, DB_NAME
./crow_server

CREATE TABLE users (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(100) NOT NULL, email VARCHAR(100) NOT NULL);

GET    /api/users       200 list
GET    /api/users/1     200, or 404
POST   /api/users       201 with the new id, or 400
PUT    /api/users/1     200, or 400, or 404
DELETE /api/users/1     204, or 404

| Method               | Use for
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------|
| `executeQuery(...)`  | Queries that return rows (e.g., `SELECT ...`) , returns  `sql::ResultSet*`                                                       |
|                      | Throws if the SQL doesn't produce a result set. Iterate with `rs->next()`.                                                       |
|                      |                                                                                                                                  |
| `executeUpdate(...)` | DML/DDL that don't return rows (`INSERT`, `UPDATE`, `DELETE`, `CREATE TABLE`…) , returns `int` (rows affected)                   |
|                      | For DDL the return is typically 0                                                                                                |
|                      | An UPDATE which doesn't change any value returns 0, even when the row exists                                                     |
|                      |                                                                                                                                  |
| `execute(...)`       | When you don't know if the SQL returns rows (e.g., stored procedures that may return rows or just an update count), returns bool |
|                      | `true` ⇒ a result set is available: call `std::unique_ptr<sql::ResultSet> rs(statement->getResultSet())`                         |
|                      |  use 'bool statement->getMoreResults()' to chk next set is available or not.                                                     |
|                      | `false` ⇒ call `statement->getUpdateCount()`.                                                                                    |
-----------------------------------------------------------------------------------------------------------------------------------------------------------

Autocommit is ON by default → every statement is committed immediately.

To make a transaction: turn autocommit OFF, run multiple statements, then commit() or rollback().


bool transferFunds(sql::Connection& conn, int from, int to, int amount) {
    try {
        conn.setAutoCommit(false); // begin
        ....
        conn.commit();              // success
        conn.setAutoCommit(true);   // restore
        return true;
    } catch (...) {
        try { conn.rollback(); } catch (...) {}
        conn.setAutoCommit(true);
        return false;
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



Data Types -> setInt, setUInt, setInt64, setUInt64, setDouble, setBoolean, setString, setDateTime, setBlob, setNull

use std::unique_ptr to store sql::PreparedStatement, sql::Statement, sql::ResultSet for avoid memory leak, else we need to delete manually

Always use prepared statements for values from a request. They stop SQL injection, as a value can never become part of the SQL.

-----------------------------------------------------------------------------------------------------------------------------------------------------------
JSON → Object ( nested example, Address needs its own macro )
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



    // All keys must be present, else get<User>() throws. This is also true for a std::optional member.
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, id, name, email)

    // A missing key keeps the value from the default constructor. A key of the wrong type still throws.
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(User, id, name, email)


    nlohmann json parsing prototype:
    	template <typename InputType>
	static nlohmann::json parse(InputType&& i,
	nlohmann::json::parser_callback_t cb = nullptr,
	bool allow_exceptions = true,
	bool ignore_comments = false);

	Version 3.12 adds a 5th argument, ignore_trailing_commas. Ubuntu's package is 3.11, where it doesn't compile.
	Comments and trailing commas aren't valid JSON, so an API shouldn't accept them.

	auto j = json::parse(s, nullptr, false);	// allow_exceptions = false, so it doesn't throw
	if (j.is_discarded()) return false; 		// check json is valid or not

-----------------------------------------------------------------------------------------------------------------------------------------------------------
Crow request details:

	req.get_header_value("content-type");		// Empty string if it is missing
	for ( const auto& header : req.headers ) ...	// Don't log them all, Authorization and Cookie hold secrets

	const char* page = req.url_params.get("page");	// nullptr if it is missing, so check before making a std::string
							// std::stoi throws on bad input, so a value from a request must be validated

	Crow turns an uncaught exception into a 500 and logs it. It doesn't crash, but bad input should get a 400.

	<int> in a route is 64-bit. A handler taking int truncates it, and id 4294967297 becomes id 1. So take std::int64_t.

	app.port(8082).concurrency(12).run();                    // blocking call, allows to specify number of threads
	app.port(8081).multithreaded().run();                    // blocking call, use std::hardware_concurrency to determine number of threads
	auto fut = app.port(8081).multithreaded().run_async();   // non blocking call, runs in async, do other work, finally call app.stop() and fut.get()

-------------------------------------------------------------------------------------------------------------------------------
*/


#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <crow.h>

#include <nlohmann/json.hpp>

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using json = nlohmann::json;


static std::string env_or(const char* name, const char* fallback) {
	const char* value = std::getenv(name);
	return value ? value : fallback;
}

inline sql::Driver* mysql_driver() { static auto* d = get_driver_instance(); return d; }

inline std::unique_ptr<sql::Connection> make_conn() {
	std::unique_ptr<sql::Connection> c(mysql_driver()->connect(
				env_or("DB_URL", "tcp://127.0.0.1:3306"),
				env_or("DB_USER", ""),
				env_or("DB_PASSWORD", "")));
	c->setSchema(env_or("DB_NAME", "testdb"));
	return c;
}

// One connection per server thread, as a connection can't be shared without a lock
static thread_local std::unique_ptr<sql::Connection> tls_conn;

// isValid() pings the server, so a restart of MySQL doesn't leave this thread with a dead connection forever.
// The ping costs one round trip per use. A pool checks only connections which were idle for a while.
inline sql::Connection& conn() {
	if (!tls_conn || !tls_conn->isValid()) tls_conn = make_conn();
	return *tls_conn;
}

class User {
	public:
		int id;
		std::string name;
		std::string email;

		User(int id = 0, std::string name = "", std::string email = "") : id(id), name(std::move(name)), email(std::move(email)) {}

		friend std::ostream& operator << (std::ostream& os, const User& user) {
			os << user.id << " , " << user.name << " , " << user.email ;
			return os;
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(User, id, name, email)
};

class UserRepository {
	public:
		virtual ~UserRepository() = default;
		virtual std::optional<User> findById(int id) = 0;
		virtual std::vector<User> findAll() = 0;
		virtual int create(const User& user) = 0;        // Returns the new id
		virtual bool update(const User& user) = 0;       // Returns false if the id doesn't exist
		virtual bool deleteById(int id) = 0;
};

class UserRepositoryImpl : public UserRepository {

	public:

		std::optional<User> findById(int id) override
		{
			std::unique_ptr<sql::PreparedStatement> stmt ( conn().prepareStatement("SELECT id, name, email FROM users WHERE id=?"));
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
			std::unique_ptr<sql::ResultSet> res (stmt->executeQuery("SELECT id, name, email FROM users"));
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

		int create(const User& user) override
		{
			sql::Connection& c = conn();
			std::unique_ptr<sql::PreparedStatement> stmt ( c.prepareStatement("INSERT INTO users (name, email) VALUES (?, ?)"));
			stmt->setString(1, user.name);
			stmt->setString(2, user.email);
			stmt->executeUpdate();

			// LAST_INSERT_ID() is per connection, so another thread's insert can't change it
			std::unique_ptr<sql::Statement> idStmt ( c.createStatement());
			std::unique_ptr<sql::ResultSet> res ( idStmt->executeQuery("SELECT LAST_INSERT_ID()"));
			res->next();
			return res->getInt(1);
		}

		bool update(const User& user) override
		{
			std::unique_ptr<sql::PreparedStatement> stmt ( conn().prepareStatement("UPDATE users SET name=?, email=? WHERE id=?"));
			stmt->setString(1, user.name);
			stmt->setString(2, user.email);
			stmt->setInt(3, user.id);
			if ( stmt->executeUpdate() == 1 )
				return true;

			// 0 rows changed means either no such id, or the values were the same already
			return findById(user.id).has_value();
		}
};


static crow::response json_response(int code, const json& body) {
	crow::response res(code, body.dump(4));
	res.set_header("Content-Type", "application/json");
	return res;
}

static crow::response error_response(int code, const std::string& message) {
	return json_response(code, json{{"error", message}});
}

// The id column is INT. A value outside it can't exist, so it is simply not found.
static bool is_valid_id(std::int64_t id) {
	return id >= 1 && id <= INT_MAX;
}

// Runs a handler, and turns an unexpected failure into an error response. The details go only to the log.
template <typename Handler>
static crow::response guarded(Handler&& handler) {
	try {
		return handler();
	} catch (const sql::SQLException& e) {
		CROW_LOG_ERROR << "Database error : " << e.what();
		return error_response(500, "database error");
	} catch (const std::exception& e) {
		CROW_LOG_ERROR << "Internal error : " << e.what();
		return error_response(500, "internal error");
	}
}

// Reads and validates the user in a request body. Returns an error response if it isn't valid.
static std::optional<crow::response> read_user(const crow::request& req, User& user) {
	static constexpr std::size_t MAX_BODY = 1 * 1024 * 1024;
	static constexpr std::size_t MAX_FIELD = 100;   // The column size

	if ( req.body.size() > MAX_BODY )
		return error_response(413, "payload too large");

	json j = json::parse(req.body, nullptr, false); // allow_exceptions = false, so it won't throw
	if ( j.is_discarded() || !j.is_object() )
		return error_response(400, "invalid json");

	try {
		user = j.get<User>();
	} catch (const json::exception&) {              // A key of the wrong type, such as {"name": 5}
		return error_response(400, "invalid field type");
	}

	if ( user.name.empty() || user.email.empty() )
		return error_response(400, "name and email are required");
	if ( user.name.size() > MAX_FIELD || user.email.size() > MAX_FIELD )
		return error_response(400, "name or email is too long");

	return std::nullopt;
}


void crow_rest_api()
{
	crow::SimpleApp app;
	UserRepositoryImpl userRepo;

	CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::GET)
		( [&userRepo] () {
			return guarded([&] {
				return json_response(200, userRepo.findAll());
			});
		} );

	CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::GET)
		( [&userRepo] (std::int64_t id) {
			return guarded([&] {
				std::optional<User> user;
				if ( is_valid_id(id) )
					user = userRepo.findById(static_cast<int>(id));
				if ( !user )
					return error_response(404, "user not found");
				return json_response(200, *user);
			});
		} );

	CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::DELETE)
		( [&userRepo] (std::int64_t id) {
			return guarded([&] {
				if ( !is_valid_id(id) || !userRepo.deleteById(static_cast<int>(id)) )
					return error_response(404, "user not found");
				return crow::response(204);
			});
		} );

	CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::POST)
		( [&userRepo] (const crow::request &req) {
			return guarded([&] {
				User user;
				if ( auto error = read_user(req, user) )
					return std::move(*error);
				if ( user.id != 0 )
					return error_response(400, "id is set by the server");

				user.id = userRepo.create(user);

				crow::response res = json_response(201, user);
				res.set_header("Location", "/api/users/" + std::to_string(user.id));
				return res;
			});
		} );

	CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::PUT)
		( [&userRepo] (const crow::request &req, std::int64_t id) {
			return guarded([&] {
				User user;
				if ( auto error = read_user(req, user) )
					return std::move(*error);
				if ( user.id != 0 && user.id != id )
					return error_response(400, "id in the body doesn't match the url");
				if ( !is_valid_id(id) )
					return error_response(404, "user not found");

				user.id = static_cast<int>(id);
				if ( !userRepo.update(user) )
					return error_response(404, "user not found");
				return json_response(200, user);
			});
		} );

	std::cout << "std::thread::hardware_concurrency() : " << std::thread::hardware_concurrency() << std::endl;

	app.port(8081).multithreaded().run();
}

int main()
{
	if ( !std::getenv("DB_USER") || !std::getenv("DB_PASSWORD") ) {
		std::cerr << "Set the DB_USER and DB_PASSWORD environment variables" << std::endl;
		return 1;
	}

	crow_rest_api();
	return 0;
}
