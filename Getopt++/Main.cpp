#include <iostream>

#define GETOPTPP_IMPL
#include "Getoptpp.h"

/*
	- Make NumberParameter truly generic, expose common types to users
	- The parse function is generic: if it contains a dot, try parsing it as a float, then as a double
	- Same for integers. Try with short, then int, then long. Use signed version only if there's a -.
*/

int main(int argc, char** argv)
{
	std::string testString;
	long testInt;
	float testFloat;

	{
		Getoptpp opt(4, "");

		opt.AddIntParam(IntegerParameter(&testInt, sizeof(int), 'i', false));
		opt.AddRealParam(RealParameter(&testFloat, sizeof(float), 'f', false));
		opt.AddStringParam(StringParameter(&testString, 0, 's', false));

		opt.Parse(argc, argv);

		int sas = 2;
	}

	return 0;
}