/******************************************************* Singleton *****************************************************
 *
 * g++ -std=c++17 singleton.cpp -o singleton
 *
 * Singleton is use to manage a resource or pool of resource, below are some example
 *
 * Logger
 * Database connections
 * Load balance
 * Configuration file
 * Cache
 * Hardware interface access
 *
 * Only 1 instance across the application
 * It should be globally accessible
 * Private constructor : others can't create an instance
 * Private destructor  : others can't delete the instance
 * Deleted copy constructor and copy assignment : nobody can copy it, not even its own members
 * Expose a wrapper static method, which use to create and access the instance
 * It can be initialized through early or lazy initialization
 *
 * Function local static (Meyers singleton) is the usual choice
 *   It is created on the first call, and C++11 guarantees that creation is thread safe
 *   It is destroyed at exit, so don't use it from the destructor of another static object
 *   It only protects the creation. Members used by many threads still need their own lock.
 *
 * Downside : it is hidden global state, which makes testing harder. Passing a reference is often better.
 *
 ***********************************************************************************************************************/


#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

// localtime() returns a shared static buffer, so it is not thread safe. localtime_r() fills the caller's buffer.
static std::string get_timestamp()
{
	std::time_t timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::tm tm_now{};
	localtime_r(&timenow, &tm_now);

	char buffer[32];
	std::strftime(buffer, sizeof(buffer), "%F %T", &tm_now);

	return std::string(buffer);
}

/********************* Singleton Class **************************/
class Singleton {

	private:

		Singleton() = default;
		~Singleton() = default;

	public:
		Singleton(const Singleton&) = delete;
		Singleton& operator= (const Singleton&) = delete;

		static Singleton& getInstance() {
			static Singleton instance;
			return instance;
		}
};

/******************** Singleton Logger, simple version, single thread only ********************************/

class SLogger
{
	private:

		std::ofstream ofs;

		~SLogger() = default;
		SLogger()
		{
			ofs.open("log.txt", std::ios::out | std::ios::app );
		}
	public:
		SLogger(const SLogger&) = delete;
		SLogger& operator = (const SLogger&) = delete;

		static SLogger& get_instance()
		{
			static SLogger logger;
			return logger;
		}

		void log(const std::string& msg)
		{
			ofs << get_timestamp() << " : " << msg << std::endl;
		}
};

enum class LogLevel {

	DEBUG = 0,
	INFO,
	WARNING,
	ERROR
};


/******************** Singleton Logger, thread safe, with log level ********************************/
class Logger
{
	private:

		std::ofstream mLogFile;
		std::mutex mLock;                                  // Guards mLogFile, mLogToConsole and the console output
		bool mLogToConsole = true;
		std::atomic<LogLevel> mLevel{LogLevel::DEBUG};     // Atomic, so filtering a message doesn't need the lock

		Logger() = default;
		~Logger() = default;                               // ofstream closes itself

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

	public:
		Logger(const Logger&) = delete;
		Logger& operator = (const Logger&) = delete;

		static Logger& get_instance()
		{
			static Logger logger;
			return logger;
		}

		void log(LogLevel level, const std::string& msg)
		{
			if ( level < mLevel.load() )
				return;

			std::string line = get_timestamp() + " [" + levelToString(level) + "] " + msg;

			std::lock_guard<std::mutex> lock(mLock);

			mLogFile << line << std::endl;
			if ( mLogToConsole )
			{
				std::cout << line << std::endl;
			}
		}

		void setLogLevel(LogLevel level)
		{
			mLevel.store(level);
		}

		void init(const std::string& filename, bool logToConsole = true)
		{
			std::lock_guard<std::mutex> lock(mLock);
			mLogToConsole = logToConsole;
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


/******  Multiton : a fixed pool of instances handed out in round robin order, not a true singleton ***********************/
class Database
{
	private:

		static constexpr unsigned int POOL_SIZE = 5;

		~Database() = default;
		Database() = default;
	public:
		Database(const Database&) = delete;
		Database& operator = (const Database&) = delete;

		static Database& get_instance()
		{
			static Database pool[POOL_SIZE];              // Thread safe creation, and destroyed at exit
			static std::atomic<unsigned int> request_count{0};

			unsigned int idx = request_count.fetch_add(1) % POOL_SIZE;
			std::cout << "Returning Database[" << idx << "] instance" << std::endl;
			return pool[idx];
		}

		void query_execute(const std::string& query)
		{
			std::cout << "Executing : " << query << std::endl;
		}
};


int main() {

	Singleton& instance = Singleton::getInstance();
	std::cout << std::boolalpha << "Same instance : " << (&instance == &Singleton::getInstance()) << std::endl;

	Logger &logger = Logger::get_instance();

	logger.init("log.txt", true);
	logger.setLogLevel(LogLevel::DEBUG);

	logger.log(LogLevel::INFO, "siva");

	for ( int i = 0; i < 10; i++ )
		logger.log(LogLevel::INFO, std::to_string(i));

	for ( int i = 0; i < 7; i++ )
		Database::get_instance().query_execute("SELECT 1");

	return 0;
}
