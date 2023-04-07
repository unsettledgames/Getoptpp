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
		Getoptpp opt("Usage message");

		// Add an optional float parameter. No errors will occur if it's not specified, otherwise testFloat will be assigned
		opt.AddParam<float>(testFloat, 'f', true);
		// Add a mandatory int parameter. Here I specified a function that ensures the specified number is in a certain range.
		opt.AddParam<int>(testInt, 'i', false, [min, max](int num) {
				if (num < min || num > max)
					return false;
				return true;
			});
		// Optional double parameter
		opt.AddParam<double>(testDouble, 'd', true);
		// Mandatory string parameter
		opt.AddParam<std::string>(testString, 's', false);
		// Mandatory flag
		opt.AddParam<bool>(testBool, 'b', false);

		// Parse the arguments and acquire their values
		opt.Parse(argc, argv);
	}

	return 0;
}