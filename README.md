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

# Dependencies
Getoptpp includes `<functional>` and `<stringstream>`. When enabling the `GETOPTP_PRINT_OUTPUT` define, `<iostream>` is also included.
