/************************** Visitor design pattern *********************************
 *
 * Visitor design pattern is a way of separating an algorithm from an object structure on which it operates
 * A practical result of this separation is the ability to add new operations to existing object structures without modifying the structures
 * It is one way to follow the open/closed principle.
 * ********************************************************************************/

#include <iostream>
#include <memory>
#include <string>
#include <vector>


class Wheel;
class Engine;
class Body;
class Car;


class IVisitor {
	public:
		virtual void visit(Wheel& wheel) = 0;
		virtual void visit(Engine& engine) = 0;
		virtual void visit(Body& body) = 0;
		virtual void visit(Car& car) = 0;
		virtual ~IVisitor() { std::cout << __func__ << std::endl; }
};


class IElement {
	public:
		virtual void accept(IVisitor& visitor) = 0;
		virtual ~IElement() { std::cout << __func__ << std::endl; }
};

class Wheel : public IElement {
	public:
		void accept(IVisitor& visitor) override { visitor.visit(*this); }
};


class Engine : public IElement {
	public:
		void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class Body : public IElement {
	public:
		void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class Car {
	private:
		std::vector<std::unique_ptr<IElement>> elements;
	public:
		//void accept(IVisitor& visitor) override { visitor.visit(*this); }
		Car()
		{
			elements.push_back(std::make_unique<Wheel>());
			elements.push_back(std::make_unique<Engine>());
			elements.push_back(std::make_unique<Body>());
		}

		std::vector<std::unique_ptr<IElement>>& get_elements() { return elements; }
};

class PrintVisitor : public IVisitor {
	public:
		void visit(Wheel& wheel) override { std::cout << "OPS -> Printing Wheel Info" << std::endl; }
		void visit(Body& body) override { std::cout << "OPS -> Printing Body Info" << std::endl; }
		void visit(Engine& engine) override { std::cout << "OPS -> Printing Engine Info" << std::endl; }
		void visit(Car& car) override {
			std::cout << "OPS -> Visiting Car Info" << std::endl;
			auto &elements = car.get_elements();
		        for ( auto &element : elements )
				element->accept(*this);	
			std::cout << "OPS -> Visited Car Info" << std::endl;
		}

};

class DoVisitor : public IVisitor {
	public:
		void visit(Wheel& wheel) override { std::cout << "OPS -> Kicking Wheel" << std::endl; }
		void visit(Body& body) override { std::cout << "OPS -> Moving Body" << std::endl; }
		void visit(Engine& engine) override { std::cout << "OPS -> Starting Engine" << std::endl; }
		void visit(Car& car) override {
			std::cout << "OPS -> Starting Car" << std::endl;
			auto &elements = car.get_elements();
		        for ( auto &element : elements )
				element->accept(*this);	
			std::cout << "OPS -> Stopped Car" << std::endl;
		}

};




int main(int argc, char *argv[], char **env)
{
	Car car;

	PrintVisitor print_visitor;
	print_visitor.visit(car);
	std::cout << std::endl;

	DoVisitor do_visitor;
	do_visitor.visit(car);
	std::cout << std::endl;

	std::unique_ptr<IElement> elements[] = { std::make_unique<Wheel>(), std::make_unique<Engine>(), std::make_unique<Body>() };
        for ( auto &element : elements )
		element->accept(print_visitor);
	std::cout << std::endl;

        for ( auto &element : elements )
		element->accept(do_visitor);
	std::cout << std::endl;

	return 0;
}

/*************************** Output ****************************
 
OPS -> Visiting Car Info
OPS -> Printing Wheel Info
OPS -> Printing Engine Info
OPS -> Printing Body Info
OPS -> Visited Car Info

OPS -> Starting Car
OPS -> Kicking Wheel
OPS -> Starting Engine
OPS -> Moving Body
OPS -> Stopped Car

OPS -> Printing Wheel Info
OPS -> Printing Engine Info
OPS -> Printing Body Info

OPS -> Kicking Wheel
OPS -> Starting Engine
OPS -> Moving Body

~IElement
~IElement
~IElement
~IVisitor
~IVisitor
~IElement
~IElement
~IElement

 **************************************************************/
