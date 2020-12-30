/******************************************************* Singleton *****************************************************
 *
 * Singleton is use to manage a resoure or pool of resource, below are some example
 *
 * Logger
 * Database connections
 * Load balance
 * Configuration file
 * Cache
 * Hardware interface access
 *
 * Only 1 instance across the appliction
 * It should be globally accessible
 * Keep constructor, destructor inside private, so that instance can't be copied by others
 * Expose a wrapper static method, which use to create and access the instance
 * It can be initialized through early or lazy initialization
 *
 ***********************************************************************************************************************/


#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>

/********************* Singleton Class **************************/
class Singleton {

	private:

		Singleton() = default;
		~Singleton() = default;
		Singleton(const Singleton&) = default;
		Singleton& operator= (Singleton&) = default;

	public:
		static Singleton& getInstance() {
			static Singleton instance;
			return instance;
		}
};

/******************** Singleton Logger ********************************/
class Logger
{
	private:
		
		std::ofstream ofs;

		~Logger() = default;
		Logger(const Logger& logger) = default;
		Logger& operator = (Logger& logger) = default;
		Logger()
		{
			ofs.open("log.txt", std::ios::out | std::ios::app );
		}
	public:
		static Logger& get_instance()
		{
			static Logger logger;
			return logger;
		}

		void log(const std::string msg)
		{
			auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ofs << ctime(&timenow) << " : " << msg << std::endl;
		}
};


/******  Singleton class for manage a pool of resource with load balance logic ***********************/
class Database
{
	private:
		
		//static const unsigned int load_balance_server_count;
		static unsigned int request_count;
		static Database* database[5];

		~Database() = default;
		Database(const Database& database) = default;
		Database& operator = (Database& database) = default;
		Database() = default;
	public:
		static Database* get_instance()
		{
			int idx = request_count++ % 5;
		        if ( database[idx] == nullptr )
			{
				database[idx] = new Database();
				std::cout << "Database[" << idx << "] instance created and returning Database[" << idx << "] instance" << std::endl;
			}
			std::cout << "Returning Database[" << idx << "] instance" << std::endl;
			return database[idx];
		}

		void query_execute(const std::string& query)
		{
		}
};

//const unsigned int Database::load_balance_server_count = 5;
unsigned int Database::request_count = 0;
Database* Database::database[5] = {nullptr};


int main() {

	Singleton& instance = Singleton::getInstance();
	
	Logger &logger = Logger::get_instance();
	logger.log("siva");
	
	for ( int i = 0; i < 10; i++ )
		logger.log(std::to_string(i));

	for ( int i = 0; i < 22; i++)
		Database *database = Database::get_instance();

	return 0;
}
