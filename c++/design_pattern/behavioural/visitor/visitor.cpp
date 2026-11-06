/************************** Visitor design pattern *********************************
 *
 * g++ -std=c++17 visitor.cpp -o visitor
 *
 * Visitor design pattern is a way of separating an algorithm from an object structure on which it operates
 * A practical result of this separation is the ability to add new operations to existing object structures without modifying the structures
 * It is one way to follow the open/closed principle.
 *
 * Double dispatch : the code which runs depends on two runtime types, the element and the visitor
 *    element->accept(visitor)  : virtual call, picks the element type
 *    visitor.visit(*this)      : virtual call, and *this has the exact type, so it picks the right overload
 *
 * Trade off :
 *    New operation    -> easy, add one new visitor class
 *    New element type -> hard, every visitor needs a new visit()
 *    So use it when the element types are stable, and the operations keep growing.
 *
 * Modern alternative : std::variant with std::visit, which needs no base class or virtual calls
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

// Car is an element made of elements. It owns the traversal, so the visitors don't repeat it.
class Car : public IElement {
	private:
		std::vector<std::unique_ptr<IElement>> elements;
	public:
		Car()
		{
			elements.push_back(std::make_unique<Wheel>());
			elements.push_back(std::make_unique<Engine>());
			elements.push_back(std::make_unique<Body>());
		}

		void accept(IVisitor& visitor) override
		{
			visitor.visit(*this);
			for ( auto &element : elements )
				element->accept(visitor);
		}
};

class PrintVisitor : public IVisitor {
	public:
		void visit(Wheel&) override { std::cout << "OPS -> Printing Wheel Info" << std::endl; }
		void visit(Body&) override { std::cout << "OPS -> Printing Body Info" << std::endl; }
		void visit(Engine&) override { std::cout << "OPS -> Printing Engine Info" << std::endl; }
		void visit(Car&) override { std::cout << "OPS -> Printing Car Info" << std::endl; }
};

class DoVisitor : public IVisitor {
	public:
		void visit(Wheel&) override { std::cout << "OPS -> Kicking Wheel" << std::endl; }
		void visit(Body&) override { std::cout << "OPS -> Moving Body" << std::endl; }
		void visit(Engine&) override { std::cout << "OPS -> Starting Engine" << std::endl; }
		void visit(Car&) override { std::cout << "OPS -> Starting Car" << std::endl; }
};


int main()
{
	Car car;

	PrintVisitor print_visitor;
	car.accept(print_visitor);
	std::cout << std::endl;

	DoVisitor do_visitor;
	car.accept(do_visitor);
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
 
OPS -> Printing Car Info
OPS -> Printing Wheel Info
OPS -> Printing Engine Info
OPS -> Printing Body Info

OPS -> Starting Car
OPS -> Kicking Wheel
OPS -> Starting Engine
OPS -> Moving Body

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
~IElement

 **************************************************************/
