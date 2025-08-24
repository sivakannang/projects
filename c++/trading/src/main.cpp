#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include "order_book.hpp"

using namespace ob;

// ---- simple timestamp for tie-breaks in examples ----
static std::uint64_t g_ts = 0;
static Order O(OrderId id, Side s, Type t, TIF tif, Price px, Qty q) {
	return Order{ id, s, t, tif, px, q, ++g_ts };
}

// ---- string helpers ----
static std::string lower(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c){ return std::tolower(c); });
	return s;
}

static void printHelp() {
	std::cout
		<< "Commands:\n"
		<< "  n <id> <side> <type> <tif> <px> <qty>   Submit order\n"
		<< "     side: buy|b | sell|s\n"
		<< "     type: limit|l | market|m\n"
		<< "     tif : gfd|day | ioc\n"
		<< "     px  : price in ticks (ignored for market)\n"
		<< "     qty : positive integer\n"
		<< "  c <id>                                  Cancel by id\n"
		<< "  p                                        Print order book\n"
		<< "  t                                        Show & clear trades\n"
		<< "  h                                        Help\n"
		<< "  q                                        Quit\n";
}

static std::optional<Side> parseSide(const std::string& s) {
	auto v = lower(s);
	if (v=="buy" || v=="b")  return Side::Buy;
	if (v=="sell"|| v=="s")  return Side::Sell;
	return std::nullopt;
}
static std::optional<Type> parseType(const std::string& s) {
	auto v = lower(s);
	if (v=="limit" || v=="l") return Type::Limit;
	if (v=="market"|| v=="m") return Type::Market;
	return std::nullopt;
}
static std::optional<TIF> parseTif(const std::string& s) {
	auto v = lower(s);
	if (v=="gfd" || v=="day") return TIF::GFD;
	if (v=="ioc")             return TIF::IOC;
	return std::nullopt;
}

// ---- pretty printers ----
static void printTradesAndClear(Book& b, const std::string& title) {
	const auto& ts = b.trades();
	std::cout << "\n--- " << title << " (" << ts.size() << " trade"
		<< (ts.size()==1? "" : "s") << ") ---\n";
	for (const auto& t : ts) {
		std::cout << "TRADE maker=" << t.maker
			<< " taker="     << t.taker
			<< " px="        << t.px
			<< " qty="       << t.qty << '\n';
	}
	if (ts.empty()) std::cout << "(no trades)\n";
	b.clearTrades();
}

static void dumpSide(const std::map<Price, PriceLevelQ, std::greater<Price>>& m) {
	for (const auto& [px, q] : m) {
		std::cout << "  " << px << " :";
		for (const auto& o : q) std::cout << " [id=" << o.id << " q=" << o.qty << "]";
		std::cout << '\n';
	}
	if (m.empty()) std::cout << "  (empty)\n";
}
static void dumpSide(const std::map<Price, PriceLevelQ>& m) {
	for (const auto& [px, q] : m) {
		std::cout << "  " << px << " :";
		for (const auto& o : q) std::cout << " [id=" << o.id << " q=" << o.qty << "]";
		std::cout << '\n';
	}
	if (m.empty()) std::cout << "  (empty)\n";
}

static void dumpBook(const Book& b) {
	// We need access to internal maps; mirror by calling const_cast and using friends?
	// To keep header clean, we’ll just reprint via the public totals across visible levels:
	// For a compact demo, we’ll reflect the idea using best levels and totals.
	// (For full visibility, consider adding const accessors to the maps in the header.)
	std::cout << "\n=== ORDER BOOK ===\n";
	std::cout << "Best Ask: ";
	if (b.hasBestAsk()) std::cout << b.bestAsk() << '\n'; else std::cout << "(none)\n";
	std::cout << "Best Bid: ";
	if (b.hasBestBid()) std::cout << b.bestBid() << '\n'; else std::cout << "(none)\n";
	std::cout << "==================\n";
}

// A slightly more verbose dump using a friend could be added, but to avoid
// changing the header we keep to best levels summary. (Tests cover depth.)

// ---- scripted demo ----
static void scriptedDemo() {
	Book book;

	auto submit = [&](const Order& o, const std::string& tag) {
		book.submit(o);
		printTradesAndClear(book, "After " + tag);
		dumpBook(book);
	};

	submit(O(1, Side::Buy,  Type::Limit, TIF::GFD, 100, 10), "NEW  BUY id=1 px=100 q=10 (rests)");
	submit(O(2, Side::Sell, Type::Limit, TIF::GFD,  99,  6), "NEW SELL id=2 px=99  q=6  (crosses)");
	submit(O(3, Side::Sell, Type::Limit, TIF::GFD, 100,  5), "NEW SELL id=3 px=100 q=5");
	submit(O(4, Side::Buy,  Type::Limit, TIF::GFD,  98,  8), "NEW  BUY id=4 px=98  q=8 (rests)");
	submit(O(5, Side::Sell, Type::Limit, TIF::GFD, 102, 12), "NEW SELL id=5 px=102 q=12 (rests)");
	submit(O(6, Side::Buy,  Type::Limit, TIF::IOC, 102, 20), "IOC BUY id=6 px=102 q=20");
	submit(O(7, Side::Sell, Type::Market, TIF::IOC,  0,  15), "MKT SELL id=7 q=15");

	// add & cancel
	book.submit(O(8, Side::Buy, Type::Limit, TIF::GFD, 99, 7));
	std::cout << "\nAdded resting BUY id=8 px=99 q=7\n";
	dumpBook(book);
	bool ok = book.cancel(8);
	std::cout << "Cancel id=8 -> " << (ok ? "OK" : "NOT FOUND") << '\n';
	dumpBook(book);
}

// ---- interactive REPL ----
static void repl() {
	Book book;
	printHelp();
	std::cout << "\nType 'h' for help. Example:\n"
		<< "  n 101 buy limit gfd 100 10\n"
		<< "  n 201 sell limit gfd  99  6\n"
		<< "  t\n  p\n";

	std::string line;
	while (true) {
		std::cout << "\n> " << std::flush;
		if (!std::getline(std::cin, line)) break;
		std::istringstream iss(line);
		std::string cmd; iss >> cmd;
		if (cmd.empty()) continue;

		auto c = lower(cmd);
		if (c == "q" || c == "quit" || c == "exit") {
			break;
		} else if (c == "h" || c == "help") {
			printHelp();
		} else if (c == "p") {
			dumpBook(book);
		} else if (c == "t") {
			printTradesAndClear(book, "Trades");
		} else if (c == "c") {
			OrderId id{};
			if (!(iss >> id)) { std::cout << "Usage: c <id>\n"; continue; }
			bool ok = book.cancel(id);
			std::cout << (ok ? "Canceled " : "Not found ") << id << '\n';
			dumpBook(book);
		} else if (c == "n") {
			// n <id> <side> <type> <tif> <px> <qty>
			std::string sideS, typeS, tifS;
			OrderId id{}; Price px{}; Qty qty{};
			if (!(iss >> id >> sideS >> typeS >> tifS >> px >> qty)) {
				std::cout << "Usage: n <id> <side> <type> <tif> <px> <qty>\n";
				continue;
			}
			auto side = parseSide(sideS);
			auto type = parseType(typeS);
			auto tif  = parseTif(tifS);
			if (!side || !type || !tif || qty <= 0) {
				std::cout << "Invalid fields. Use 'h' for help.\n";
				continue;
			}
			Order o = O(id, *side, *type, *tif, px, qty);
			book.submit(o);
			printTradesAndClear(book, "Submit result");
			dumpBook(book);
		} else {
			std::cout << "Unknown command. 'h' for help.\n";
		}
	}
}

int main(int argc, char** argv) {
	if (argc > 1 && std::string(argv[1]) == "--script") {
		scriptedDemo();
	} else {
		repl();
	}
	return 0;
}

