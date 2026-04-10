#include <iostream>
#include "shared_ptr.hpp"

struct Widget
{
	~Widget()
	{
		std::cout << "Destructor\n";
	}
};

int main()
{
	shared_ptr<Widget> a = new Widget,
					   b = a,
					   c = a,
					   d = a;
}