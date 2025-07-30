// g++ crow_rest.cpp -o crow_server -I/mnt/d/sdk/Crow/include -I/mnt/d/sdk/json/include -lpthread -lmysqlcpplient
//


#include <iostream>
#include <string>
#include <memory>

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

#include <crow.h>

#include <nlohmann/json.hpp>


class MySqlHelper {

	std::shared_ptr<sql::Connection> conn;

	public:
		MySqlHelper(std::string url, std::string user, std::string pass, std::string dbname)
		{
			sql::Driver *driver = get_driver_instance();

			conn = std::shared_ptr<sql::Connection>(driver->connect(url, user, pass));
			conn->setSchema(dbname);
		}
};

class User {
	public:
		int id;
		std::string name;
		std::string email;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, id, name, email)
};

int main()
{
	MySqlHelper db("tcp://127.0.0.1:3306", "siva", "kannan", "testdb");
	return 0;
}
