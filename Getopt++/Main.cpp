#include <iostream>

#define GETOPTPP_IMPL
#include "Getoptpp.h"

/*
	- Parse and validate should both be called by Getoptpp. In case of error it's possible to notify and print the error message (only print it once pls)
*/

int main(int argc, char** argv)
{
	std::string testString;
	long testInt;
	float testFloat;

	{
		Getoptpp opt(4, "");

		opt.AddNumberParam<int>({ &testInt, 'i', false, [](void* data) {
				return *(int*)data == 15;
			}});
		opt.AddNumberParam<float>({ &testFloat, 'f', false });
		opt.AddStringParam(StringParameter(&testString, 0, 's', false));

		opt.Parse(argc, argv);

		int sas = 2;
	}

	return 0;
}