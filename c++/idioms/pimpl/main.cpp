#include "API.h"

#include <utility>

int main()
{
	API api(1, 2);
	api.execute();

	API copy(api);              // Deep copy, so changing the copy doesn't change api
	copy.set_a(10);
	copy.execute();
	api.execute();

	API moved(std::move(api));  // api can only be destroyed or assigned now
	moved.execute();

	api = copy;                 // Copy assignment gives api a new implementation
	api.execute();

	return 0;
}
