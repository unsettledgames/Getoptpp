# Getoptpp
> C++ header-only wrapper and abstraction layer around getopt.

# Integration
- Include <Getoptpp.h> in your project

# Usage
- Declare a `Getoptpp` object
- Add the parameters you need using the `AddParam` method of your Getoptpp object
- Call the `Parse` method of your Getoptpp object to load the command line arguments into the specified variables

For each type of parameter, you have to specify three values: a reference to the variable that will hold the parsed data, a char that represents 
its option string and a boolean that specifies whether the parameter is optional or not. It's also possible to pass an optional std::function that 
acts as a validator: given a reference to the data, it should return true when all the desired conditions are met, false otherwise. 
When a validator fails inside a `Parse()` call, GETOPTPP_ERR_INVALID_VALUE is returned. For more error types, see the top of Getoptpp.h. 

Example code:

```
int testInt;
int min = -5, max = 10;
int nOptions = 1;

Getoptpp optObj(nOptions, "");

obtObj.AddParam<int>(testInt, 'i', false, [min, max](void* data) {
  int num = *(int*)data;
  if (num < min || num > max)
    return false;
  else
    return true;
});

obtObj.Parse(argc, argv);
```

Optionally, you can `#define GETOPTP_PRINT_OUTPUT` to print error / status messages from Getoptpp to the terminal.

# Contributing
For any kind of bugs, please open an issue and I'll try to address it. Feel free to also create pull requests that fix bugs, if you feel like it. Extensions and additional features are welcome, but I'd prefer to discuss them in an issue before getting to a PR.

# Dependencies
Getoptpp includes `<functional>`, `<vector>`, `<memory>` and `<stringstream>`. When enabling the `GETOPTP_PRINT_OUTPUT` define, `<iostream>` is also included.
