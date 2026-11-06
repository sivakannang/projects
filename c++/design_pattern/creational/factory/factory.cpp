/*************************** Factory Design Pattern ****************
 *
 * g++ -std=c++17 factory.cpp -o factory
 *
 * Problem :
 *    We want to decide at run time what object is to be created based on some configuration or application parameter. When we write the code, we do not know what class should be instantiated.
 *
 * Three different things are called factory :
 *    Simple Factory   : One function which picks the class, usually with a switch. This sample shows this one.
 *                       Easy to read, but every new class means changing that switch, which breaks the open/closed principle.
 *    Factory Method   : A base class declares a virtual create(), and its subclasses decide which class to instantiate.
 *    Abstract Factory : An interface which creates a family of related objects, such as a matching socket and parser per exchange.
 *
 * *****************************************************************/


#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

class Connection {
protected:
	std::string cls;
public:
	virtual ~Connection() = default;
	virtual void connect() = 0;
	virtual void write(const std::string& msg) = 0;
	virtual void read() = 0;
	virtual void disconnect() = 0; 
};

class Gprs : public Connection {

public:
	Gprs() { cls = __func__ ; }
	void connect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void disconnect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void write(const std::string& msg) override { std::cout << cls << "::" << __func__ << " -> " << msg << std::endl; }
	void read() override { std::cout << cls << "::" << __func__ << std::endl; }
};


class Wifi : public Connection {

public:
	Wifi() { cls = __func__ ; }
	void connect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void disconnect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void write(const std::string& msg) override { std::cout << cls << "::" << __func__ << " -> " << msg << std::endl; }
	void read() override { std::cout << cls << "::" << __func__ << std::endl; }
};

class ConnectionFactory {

public:
	enum class ConnectionProtocol
	{
		WIFI,
		GPRS,
		BLUETOOTH,
		ETHERNET,
		SERIAL
	};

	static std::unique_ptr<Connection> get_connection(ConnectionFactory::ConnectionProtocol protocol)
	{
		switch(protocol)
		{
			case ConnectionProtocol::GPRS      : return std::make_unique<Gprs>();
			case ConnectionProtocol::WIFI      : return std::make_unique<Wifi>();
			case ConnectionProtocol::BLUETOOTH : throw std::runtime_error("BLUETOOTH connection not yet implemented");
			case ConnectionProtocol::SERIAL    : throw std::runtime_error("SERIAL connection not yet implemented");
			case ConnectionProtocol::ETHERNET  : throw std::runtime_error("ETHERNET connection not yet implemented");
			default                            : throw std::invalid_argument("Undefined Connection");
		}
	}
};


int main()
{
	auto connection = ConnectionFactory::get_connection(ConnectionFactory::ConnectionProtocol::GPRS);
	connection->connect();
	connection->write("Packet sending.....");
	connection->read();
	connection->disconnect();

	try {
		ConnectionFactory::get_connection(ConnectionFactory::ConnectionProtocol::SERIAL);
	} catch ( const std::exception& e ) {
		std::cout << "Error : " << e.what() << std::endl;
	}

	return 0;
}
