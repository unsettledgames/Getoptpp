#define GETOPTPP_IMPL
#define GETOPTPP_PRINT_OUTPUT
#include "Getoptpp.h"

int main(int argc, char** argv)
{
	int min = 0;
	int max = 18;

	std::string testString;
	int testInt;
	float testFloat;
	double testDouble;
	bool testBool;

	// I put everything in a scope to make sure data doesn't get corrupted after parameters and the Getoptpp object are destroyed
	{
		Getoptpp opt(5, "Usage message");

		// Add an optional float parameter. No errors will occur if it's not specified, otherwise testFloat will be assigned
		opt.AddNumberParam<float>({ &testFloat, 'f', true });
		// Add a mandatory int parameter. Here I specified a function that ensures the specified number is in a certain range.
		opt.AddNumberParam<int>({ &testInt, 'i', false, [min, max](void* data) {
				int num = *(int*)data;
				if (num < min || num > max)
					return false;
				return true;
			}});
		// Optional double parameter
		opt.AddNumberParam<double>({ &testDouble, 'd', true });
		// Mandatory string parameter
		opt.AddStringParam({ &testString, 's', false });
		// Mandatory flag
		opt.AddBoolParam({ &testBool, 'b', false });

		// This parameter will print an error: it is the sixth one, but we allocated space for 5 parameters at the beginning
		opt.AddBoolParam({ &testBool, 'b', false });

		// Parse the arguments and acquire their values
		opt.Parse(argc, argv);
	}

	return 0;
}