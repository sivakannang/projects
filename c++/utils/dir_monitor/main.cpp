// main.cpp
#include "DirectoryWatcher.hpp"
#include <iostream>
#include <thread>
#include <csignal>

static std::atomic<bool> g_stop{false};
void signal_handler(int) { g_stop = true; }

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <dir>\n";
		return 1;
	}

	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	DirectoryWatcher watcher(argv[1]);

	watcher.setCallback([](const FileEvent& e){
			std::string t;
			switch (e.type) {
			case FileEventType::Create: t = "CREATE"; break;
			case FileEventType::Delete: t = "DELETE"; break;
			case FileEventType::Modify: t = "MODIFY"; break;
			case FileEventType::Move:   t = "MOVE"; break;
			case FileEventType::CloseWrite: t = "CLOSE_WRITE"; break;
			default: t = "UNKNOWN"; break;
			}

			std::cout << "[" << t << "] "
			<< (e.isDirectory ? "(DIR) " : "(FILE) ")
			<< e.path;

			if (e.type == FileEventType::Move)
			std::cout << "  (from: " << e.oldPath << ")";

			std::cout << "\n";
			});

	std::thread th([&]() { watcher.run(); });

	while (!g_stop)
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	watcher.stop();
	th.join();
}

