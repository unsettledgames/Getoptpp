# Getoptpp
> C++ header-only wrapper and abstraction layer around getopt.

# Integration
- Add Getoptpp.h in your project
- In one and only one cpp file, before including Getoptpp.h, define GETOPTPP_IMPL

Here's the content of an example .cpp file:

```
#define GETOPTPP_IMPL
#include "Getoptpp.h"

// You can now use Getoptpp

```
# Usage
- Declare a `Getoptpp` object
- Add the parameters you need using the `Add[Type]Param` methods of your Getoptpp object
- Call the `Parse` method of your Getoptpp object to load the command line arguments into the specified variables

Parameters can either be declared and then passed or created using initialization lists. The following blocks achieve the same results.
Using explicit declaration:
```
int testInt;
int nOptions = 1;

Getoptpp optObj(nOptions, "");
NumberParam<int> intParam(&testInt, 'i', false);

obtObj.AddNumberParam<int>(intParam);
obtObj.Parse();
```
Using initialization list:
```
int testInt;

Getoptpp optObj(nOptions, "");

obtObj.AddNumberParam<int>({ &testInt, 'i', false});
obtObj.Parse();

```

For each type of parameter (except boolean flags) it's possible to pass an std::function that acts as a validator: given a pointer to the data, it should return true when all the desired conditions are met, false otherwise. When a validator fails inside a `Parse()` call, GETOPTPP_ERR_INVALID_VALUE is returned. For more error types, see the top of Getoptpp.h. 

```
int testInt;
int min = -5, max = 10;
int nOptions = 1;

Getoptpp optObj(nOptions, "");
NumberParam<int> intParam(&testInt, 'i', false);

obtObj.AddNumberParam<int>({&testInt, 'i', false, [min, max](void* data) {
  int num = *(int*)data;
  if (num < min || num > max)
    return false;
  else
    return true;
}});

obtObj.Parse();
```
Optionally, you can `#define GETOPTP_PRINT_OUTPUT` to print error / status messages from Getoptpp to the terminal.

# Contributing
For any kind of bugs, please open an issue and I'll try to address it. Feel free to also create pull requests that fix bugs, if you feel like it. Extensions and additional features are welcome, but I'd prefer to discuss them in an issue before getting to a PR.

# Dependencies
Getoptpp includes `<functional>` and `<stringstream>`. When enabling the `GETOPTP_PRINT_OUTPUT` define, `<iostream>` is also included.
