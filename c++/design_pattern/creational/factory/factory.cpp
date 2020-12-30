/*************************** Factory Design Pattern ****************
 *
 * Factory          : A utility class that creates an instance of a class from a family of derived classes
 * Abstract Factory : A utility class that creates an instance of several families of classes. It can also return a factory for a certain group.
 *
 * Problem :
 *    We want to decide at run time what object is to be created based on some configuration or application parameter. When we write the code, we do not know what class should be instantiated.
 *
 * Solution:
 *    Define an interface for creating an object, but let subclasses decide which class to instantiate. Factory Method lets a class defer instantiation to subclasses.
 *   
 * *****************************************************************/


#include <iostream>
#include <memory>
#include <string>
#include <exception>

class Connection {
protected:
	std::string cls;
public:
	virtual void connect() = 0;
	virtual void write(std::string msg) = 0;
	virtual void read() = 0;
	virtual void disconnect() = 0; 
};

class Gprs : public Connection {

public:
	Gprs() { cls = __func__ ; }
	void connect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void disconnect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void write(std::string msg) override { std::cout << cls << "::" << __func__ << " -> " << msg << std::endl; }
	void read() override { std::cout << cls << "::" << __func__ << std::endl; }
};


class Wifi : public Connection {

public:
	Wifi() { cls = __func__ ; }
	void connect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void disconnect() override { std::cout << cls << "::" << __func__ << std::endl; }
	void write(std::string msg) override { std::cout << cls << "::" << __func__ << " -> " << msg << std::endl; }
	void read() override { std::cout << cls << "::" << __func__ << std::endl; }
};

class ConnectionFactory {

public:
	enum ConnectionProtocol
	{
		WIFI,
		GPRS,
		BLUETOOTH,
		ETHERNET,
		SERIAL
	};

	std::unique_ptr<Connection> get_connection(ConnectionFactory::ConnectionProtocol protocol)
	{
		switch(protocol)
		{
			case ConnectionProtocol::GPRS      : return std::make_unique<Gprs>();
			case ConnectionProtocol::WIFI      : return std::make_unique<Wifi>();
			case ConnectionProtocol::BLUETOOTH : throw "BLUETOOTH connection not yet implemented" ; 
			case ConnectionProtocol::SERIAL    : throw "SERIAL connection not yet implemented" ;
			case ConnectionProtocol::ETHERNET  : throw "ETHERNET connection not yet implemented" ; 
			default                            : throw "Undefined Connection" ;

		}
	}
};


int main(int argc, char *argv[], char **env)
{
	auto connection_factory = std::make_unique<ConnectionFactory>();
	auto connection         = connection_factory->get_connection(connection_factory->ConnectionProtocol::GPRS);
	connection->connect();
	connection->write("Packet sending.....");
	connection->disconnect();
	return 0;
}
