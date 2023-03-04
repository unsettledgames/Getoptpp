#include <iostream>

#define GETOPTPP_IMPL
#include "Getoptpp.h"

/*
*	- Call validate in number!
	- Parse and validate should both be called by Getoptpp. In case of error it's possible to notify and print the error message (only print it once pls)
*/

int main(int argc, char** argv)
{
	int min = 0;
	int max = 18;

	std::string testString;
	int testInt;
	float testFloat;
	double testDouble;

	{
		Getoptpp opt(4, "");

		opt.AddNumberParam<int>({ &testInt, 'i', false, [min, max](void* data) {
				int num = *(int*)data;
				if (num < min || num > max)
					std::cout << "Lambda ok" << std::endl;
				return *(int*)data == 15;
			}});
		opt.AddNumberParam<float>({ &testFloat, 'f', false });
		opt.AddNumberParam<double>({ &testDouble, 'd', false });
		opt.AddStringParam(StringParameter(&testString, 0, 's', false));

		opt.Parse(argc, argv);

		int sas = 2;
	}

	return 0;
}