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
#include <thread>
#include <mutex>

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

class SLogger
{
	private:

		std::ofstream ofs;

		~SLogger() = default;
		SLogger(const SLogger& logger) = default;
		SLogger& operator = (const SLogger& logger) = default;
		SLogger()
		{
			ofs.open("log.txt", std::ios::out | std::ios::app );
		}
	public:
		static SLogger& get_instance()
		{
			static SLogger logger;
			return logger;
		}

		void log(const std::string msg)
		{
			auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ofs << ctime(&timenow) << " : " << msg << std::endl;
		}
};

enum class LogLevel {

	DEBUG = 0,
	INFO,
	WARNING,
	ERROR
};


class Logger
{
	private:
		
		std::ofstream mLogFile;
		std::mutex mLock;
		bool mLogToConsole = true;
		LogLevel mLevel = LogLevel::DEBUG;

		Logger() = default;
		
		~Logger()
		{
			if (mLogFile.is_open() )
			{
				mLogFile.close();
			}
		}
		
		Logger(const Logger& logger) = default;
		Logger& operator = (const Logger& logger) = default;

		std::string levelToString(LogLevel level)
		{
			switch(level)
			{
				case LogLevel::DEBUG    : return "DEBUG";
				case LogLevel::INFO     : return "INFO";
				case LogLevel::WARNING  : return "WARNING";
				case LogLevel::ERROR    : return "ERROR";
			}
			return "UNKNOWN";
		}

		std::string getTimestamp()
		{
			auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			char buffer[32];

			std::strftime(buffer, sizeof(buffer), "%F %T", std::localtime(&timenow));

			return std::string(buffer);

		}

	public:
		static Logger& get_instance()
		{
			static Logger logger;
			return logger;
		}

		void log(LogLevel level, const std::string& msg)
		{
			if ( level < mLevel )
				return;

			std::string timestamp = getTimestamp();
			
			std::lock_guard<std::mutex> lock(mLock);

			mLogFile << timestamp << " [" << levelToString(level) << "] " << msg << std::endl;
			if ( mLogToConsole )
			{
				std::cout << timestamp << " [" << levelToString(level) << "] " << msg << std::endl;
			}
		}

		void setLogLevel(LogLevel level)
		{
			std::lock_guard<std::mutex> lock(mLock);
			mLevel = level;
		}

		void init(const std::string& filename, bool logToConsole = true)
		{
			std::lock_guard<std::mutex> lock(mLock);
			if ( !mLogFile.is_open() )
			{
				mLogFile.open(filename, std::ios::app);
				if ( !mLogFile.is_open() )
				{
					throw std::runtime_error("Failed to open log file");
				}
			}
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
	
	logger.init("log.txt", true);
	logger.setLogLevel(LogLevel::DEBUG);

	logger.log(LogLevel::INFO, "siva");
	
	for ( int i = 0; i < 10; i++ )
		logger.log(LogLevel::INFO, std::to_string(i));

	for ( int i = 0; i < 22; i++)
		Database *database = Database::get_instance();

	return 0;
}
