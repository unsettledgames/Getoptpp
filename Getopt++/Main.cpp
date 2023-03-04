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

	{
		Getoptpp opt(5, "Usage message");

		opt.AddNumberParam<float>({ &testFloat, 'f', false });
		opt.AddNumberParam<int>({ &testInt, 'i', false, [min, max](void* data) {
				int num = *(int*)data;
				if (num < min || num > max)
					std::cout << "Lambda ok" << std::endl;
				return *(int*)data == 15;
			}});
		opt.AddNumberParam<double>({ &testDouble, 'd', false });
		opt.AddStringParam({ &testString, 's', false });
		opt.AddBoolParam({ &testBool, 'b' });

		opt.Parse(argc, argv);

		int sas = 2;
	}

	return 0;
}