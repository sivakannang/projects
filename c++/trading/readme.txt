Folder Layout :

.
├─ CMakeLists.txt
├─ inc/
│  └─ order_book.hpp
├─ src/
│  ├─ order_book.cpp
│  └─ main.cpp
└─ tests/
   └─ test_order_book.cpp


cmake -S . -B build
cmake --build build -j

# demo
./build/order_book_demo --script   # scripted
./build/order_book_demo            # REPL

# tests (if BUILD_TESTING=ON, default)
ctest --test-dir build --output-on-failure

