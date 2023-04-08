#ifndef GETOPTPP_H
#define GETOPTPP_H

#ifdef GETOPTPP_PRINT_OUTPUT
#include <iostream>
#endif

#include <sstream>
#include <vector>
#include <memory>
#include <functional>

// No errors occurred during the parsing
#define GETOPTPP_ERR_NO_ERROR					0
// The user specified too many argument
#define GETOPTPP_ERR_TOO_MANY_ARGUMENTS			-1
// The user didn't specify a non-optional parameter
#define GETOPTPP_ERR_UNSPECIFIED_NON_OPTIONAL	-2
// The user requested the -h or -? option, which prints the usage message, no additional arguments were parsed
#define GETOPTPP_ERR_ASKED_USAGE				-3
// The user submitted an unknown option
#define GETOPTPP_ERR_UNKNOWN_OPTION				-4

// There was an error parsing a parameter. This can happen when the user specifies a value that has a different type than 
// the variable that should contain it
#define GETOPTPP_ERR_PARSE_ERROR				-5
// The validator function for a certain parameter returned false
#define GETOPTPP_ERR_INVALID_VALUE				-6

typedef int GetoptParseError;

#ifndef _WIN32
#include <unistd.h>
#else
static int opterr = 1, optind = 1, optopt, optreset;
static const char* optarg;
static inline int getopt(int nargc, char* const nargv[], const char* ostr)
{
	static const char* place = "";        // option letter processing
	const char* oli;                      // option letter list index

	// update scanning pointer
	if (optreset || !*place)
	{
		optreset = 0;
		if (optind >= nargc || *(place = nargv[optind]) != '-')
		{
			place = "";
			return -1;
		}

		while (*place == '-')
			place++;
	}

	// option letter okay?
	if ((optopt = (int)*place++) == (int)':' || !(oli = strchr(ostr, optopt)))
	{
		// if the user didn't specify '-' as an option,  assume it means -1.
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;
#ifdef GETOPTPP_PRINT_OUTPUT
		if (opterr && *ostr != ':' && optopt != 'h' && optopt != '?')
			std::cout << "illegal option -" << (char)optopt << "\n";
#endif
		return ('?');
	}

	if (*++oli != ':')
	{                    // don't need argument
		optarg = NULL;
		if (!*place)
			++optind;
	}
	else
	{                                // need an argument
		if (*place)                         // no white space
			optarg = place;
		else if (nargc <= ++optind)
		{       // no arg
			place = "";
			if (*ostr == ':')
				return (':');
#ifdef GETOPTPP_PRINT_OUTPUT
			if (opterr)
				std::cout << "option requires an argument -- " << optopt << "\n";
#endif
			return (':');
		}
		else                              // white space
			optarg = nargv[optind];
		place = "";
		++optind;
	}
	return optopt;                          // dump back option letter
}
#endif

/**
*	Parameter: base class used to represent a command-line argument. A parameter needs:
*		- A reference to variable that holds the value of that parameter
*		- The character used by getopt to recognize it
*		- A boolean that specifies whether the parameter is a flag or not (whether it accepts an option or not)
*		- An optional validation function, that is called to ensure that the obtained value respects some user-defined constraints
*/

class ParameterBase
{
	friend class Getoptpp;

public:
	inline ParameterBase() = default;
	inline virtual ~ParameterBase() = default;
	inline ParameterBase(char c, bool optional, bool isFlag) : m_Char(c), m_Optional(optional), m_IsFlag(isFlag) {}

	inline virtual GetoptParseError Parse(std::string s)
	{ 
		return GETOPTPP_ERR_NO_ERROR; 
	}

protected:
	char m_Char;
	bool m_Optional;
	bool m_IsFlag;	
};

template<typename T>
class Parameter : public ParameterBase
{
public:
	inline Parameter() = default;
	inline Parameter(T& data, char c, bool optional, bool isFlag, std::function<bool(T)> validator = {}) :
		ParameterBase(c, optional, isFlag), m_Data(data), m_Validator(validator) {}

	inline virtual ~Parameter() = default;
	
	inline virtual GetoptParseError Parse(std::string s) override
	{
		if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>)
		{
			std::istringstream iss(s.c_str());
			iss >> m_Data;

			if (iss.fail())
			{
#ifdef GETOPTPP_PRINT_OUTPUT
				std::cout << "Error parsing option -" << m_Char << std::endl;
#endif
				return GETOPTPP_ERR_PARSE_ERROR;
			}
		}
		else
			m_Data = s;

		if (m_Validator)
		{
			if (m_Validator(m_Data))
				return GETOPTPP_ERR_NO_ERROR;
			else
				return GETOPTPP_ERR_INVALID_VALUE;
		}
	}

private:
	T& m_Data;
	std::function<bool(T)> m_Validator;
};

class Getoptpp
{
public:
	Getoptpp(const std::string& helpMessage = "") : m_HelpMessage(helpMessage) {}

	inline GetoptParseError Parse(int argc, char** argv)
	{
		GetoptParseError ret = GETOPTPP_ERR_NO_ERROR;

		bool printUsage = false;
		std::string optStr = "";
		std::string notOptional = "";
		std::string bools = "";

		for (uint32_t i = 0; i < m_Parameters.size(); i++)
		{
			optStr += m_Parameters[i]->m_Char;
			if (!m_Parameters[i]->m_IsFlag)
				optStr += ":";
			if (!m_Parameters[i]->m_Optional)
				notOptional += m_Parameters[i]->m_Char;
		}

		int c;

		while ((c = getopt(argc, argv, optStr.c_str())) != -1)
		{
			if (c == 'h' || c == '?')
			{
				ret = GETOPTPP_ERR_ASKED_USAGE;
				printUsage = true;
				break;
			}

			bool found = false;
			for (uint32_t i = 0; i < m_Parameters.size() && !found; i++)
			{
				if (m_Parameters[i]->m_Char == c)
				{
					if (!m_Parameters[i]->m_Optional)
						notOptional[notOptional.find(m_Parameters[i]->m_Char)] = '?';
					found = true;

					m_Parameters[i]->Parse(optarg);
				}
			}

			if (!found)
			{
#ifdef GETOPTPP_PRINT_OUTPUT
				std::cerr << "Unknown option: " << (char)c << std::endl;
				printUsage = true;
#endif
				ret = GETOPTPP_ERR_UNKNOWN_OPTION;
			}
		}

		bool allFound = true;
		for (uint32_t i = 0; i < notOptional.length() && allFound; i++)
		{
			if (notOptional[i] != '?')
			{
				allFound = false;
#ifdef GETOPTPP_PRINT_OUTPUT
				std::cout << "Unspecified non-optional argument -" << notOptional[i] << std::endl;
#endif
				ret = GETOPTPP_ERR_UNSPECIFIED_NON_OPTIONAL;
			}
		}


		if (optind > argc)
		{
			if (allFound)
			{
#ifdef GETOPTPP_PRINT_OUTPUT
#ifdef _WIN32
				std::cerr << "Too many arguments or argument before other options\n";
#else
				std::cerr << "Too many arguments\n";
#endif
				printUsage = true;
#endif
				ret = GETOPTPP_ERR_TOO_MANY_ARGUMENTS;
			}
		}

#ifdef GETOPTPP_PRINT_OUTPUT
		if (printUsage)
			std::cout << m_HelpMessage << std::endl;
#endif

		return ret;
	}

	template<typename T>
	inline void AddParam(T& data, char c, bool optional, std::function<bool(T)> validator = {})
	{
		m_Parameters.push_back(std::make_unique<Parameter<T>>(data, c, optional, std::is_same<bool, T>(), validator));
	}

private:
	std::string m_HelpMessage;
	std::vector<std::unique_ptr<ParameterBase>> m_Parameters;

};

#endif