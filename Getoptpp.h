#ifndef GETOPTPP_H
#define GETOPTPP_H

#ifdef GETOPTPP_PRINT_OUTPUT
#include <iostream>
#endif

#include <sstream>
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

#ifdef GETOPTPP_PRINT_OUTPUT
#define CheckParamArraySize()		if (m_CurrSize >= m_MaxSize)																	 \
									{																								 \
										std::cout << "Couldn't add parameter, the number of already existing parameters exceeded "	 \
											<< "the maximum specified when creating the Getoptpp object." << std::endl;				 \
										return;																						 \
									}
#else
#define CheckParamArraySize()		if (m_CurrSize >= m_MaxSize)																	 \
										return;																						 
#endif

#ifndef _WIN32
#include <unistd.h>
#else
int opterr = 1, optind = 1, optopt, optreset;
const char* optarg;
int getopt(int nargc, char* const nargv[], const char* ostr);
#endif

/**
*	Parameter: base class used to represent a command-line argument. A parameter needs:
*		- A pointer to variable that holds the value of that parameter
*		- The size of said data
*		- The character used by getopt to recognize it
*		- A boolean that specifies whether the parameter is a flag or not (whether it accepts an option or not)
*		- An optional validation function, that is called to ensure that the obtained value respects some user-defined constraints
*/
class Parameter
{
	friend class Getoptpp;

public:
	Parameter() = default;
	virtual ~Parameter() = default;
	Parameter(void* data, size_t size, char c, bool optional, bool isFlag, std::function<bool(void*)> validator = {}) :
		m_Data(data), m_DataSize(size), m_Char(c), m_Optional(optional), m_IsFlag(isFlag), m_Validator(validator) {}

	virtual GetoptParseError Parse(void* data) { return GETOPTPP_ERR_NO_ERROR; }

protected:
	char m_Char;
	bool m_Optional;
	bool m_IsFlag;

	void* m_Data;
	size_t m_DataSize;
	std::function<bool(void*)> m_Validator;
};

class StringParameter : public Parameter
{
public:
	StringParameter() = default;
	~StringParameter() = default;
	StringParameter(void* data, char c, bool optional, std::function<bool(void*)> validator = {});

	GetoptParseError Parse(void* data) override;
};

template <typename T>
class NumberParameter : public Parameter
{
public:
	NumberParameter() = default;
	~NumberParameter() = default;
	NumberParameter(void* data, char c, bool optional, std::function<bool(void*)> validator = {});
	
	GetoptParseError Parse(void* data) override;
};

class BoolParameter : public Parameter
{
	friend class Getoptpp;
public:
	BoolParameter() = default;
	~BoolParameter() = default;
	BoolParameter(void* data, char c, bool optional);

	GetoptParseError Parse(void* data) override;
};

class Getoptpp
{
public:
	Getoptpp(uint32_t nOptions, const std::string& helpMessage = "");
	~Getoptpp();

	GetoptParseError Parse(int argc, char** argv);

	void AddStringParam(StringParameter p);
	void AddBoolParam(BoolParameter p);
	template<typename T>
	void AddNumberParam(NumberParameter<T> param);

private:
	std::string m_HelpMessage;
	Parameter** m_Parameters;

	uint32_t m_CurrSize;
	uint32_t m_MaxSize;

};

#ifdef GETOPTPP_IMPL

#ifdef _WIN32
int getopt(int nargc, char* const nargv[], const char* ostr) {
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
		if (opterr && *ostr != ':')
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

/************************************************************ STRING PARAMETER ***********************************************************/

GetoptParseError StringParameter::Parse(void* data)
{
	std::string param = std::string((char*)data);
	*(std::string*)m_Data = param;

	if (m_Validator)
	{
		if (m_Validator(m_Data))
			return GETOPTPP_ERR_NO_ERROR;
		else
			return GETOPTPP_ERR_INVALID_VALUE;
	}

	return GETOPTPP_ERR_NO_ERROR;
}

StringParameter::StringParameter(void* data, char c, bool optional, std::function<bool(void*)> validator) :
	Parameter(data, 0, c, optional, false, validator) {}

/************************************************************ NUMBER PARAMETER ***********************************************************/

template<typename T>
NumberParameter<T>::NumberParameter(void* data, char c, bool optional, std::function<bool(void*)> validator) :
	Parameter(data, sizeof(T), c, optional, false, validator) {}

template <typename T>
GetoptParseError NumberParameter<T>::Parse(void* data)
{
	T tmp;
	std::istringstream iss((char*)data);
	iss >> tmp;

	if (iss.fail())
	{
#ifdef GETOPTPP_PRINT_OUTPUT
		std::cout << "Error parsing option -" << m_Char << std::endl;
#endif
		return GETOPTPP_ERR_PARSE_ERROR;
	}

	memcpy(m_Data, &tmp, m_DataSize);
	if (m_Validator)
	{
		if (m_Validator(m_Data))
			return GETOPTPP_ERR_NO_ERROR;
		else
			return GETOPTPP_ERR_INVALID_VALUE;
	}

	return GETOPTPP_ERR_NO_ERROR;
}

/************************************************************ BOOL PARAMETER ***********************************************************/

BoolParameter::BoolParameter(void* data, char c, bool optional) : Parameter(data, sizeof(bool), c, optional, true, {}) {}

GetoptParseError BoolParameter::Parse(void* data) 
{
	*(bool*)m_Data = true;
	return GETOPTPP_ERR_NO_ERROR; 
}

/************************************************************ GETOPTPP ***********************************************************/


Getoptpp::Getoptpp(uint32_t nOptions, const std::string& helpMessage) : m_HelpMessage(helpMessage)
{
	m_CurrSize = 0;
	m_MaxSize = nOptions;

	m_Parameters = new Parameter*[nOptions];
	for (uint32_t i = 0; i < nOptions; i++)
		m_Parameters[i] = nullptr;
}

Getoptpp::~Getoptpp()
{
	for (uint32_t i = 0; i < m_CurrSize; i++)
		if (m_Parameters[i] != nullptr)
			delete m_Parameters[i];
	delete[] m_Parameters;
}

GetoptParseError Getoptpp::Parse(int argc, char** argv)
{
	GetoptParseError ret = GETOPTPP_ERR_NO_ERROR;

	bool printUsage = false;
	std::string optStr = "";
	std::string notOptional = "";
	std::string bools = "";

	for (uint32_t i = 0; i < m_CurrSize; i++)
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
		for (uint32_t i = 0; i < m_CurrSize && !found; i++)
		{
			if (m_Parameters[i]->m_Char == c)
			{
				if (!m_Parameters[i]->m_Optional)
					notOptional[notOptional.find(m_Parameters[i]->m_Char)] = '?';
				found = true;

				m_Parameters[i]->Parse((void*)optarg);
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
void Getoptpp::AddNumberParam(NumberParameter<T> p)
{
	CheckParamArraySize();
	NumberParameter<T>* toAdd = new NumberParameter<T>(p.m_Data, p.m_Char, p.m_Optional, p.m_Validator);

	m_Parameters[m_CurrSize] = toAdd;
	m_CurrSize++;
}

void Getoptpp::AddStringParam(StringParameter p)
{
	CheckParamArraySize();
	StringParameter* toAdd = new StringParameter(p.m_Data, p.m_Char, p.m_Optional, p.m_Validator);

	m_Parameters[m_CurrSize] = toAdd;
	m_CurrSize++;
}

void Getoptpp::AddBoolParam(BoolParameter p)
{
	CheckParamArraySize();
	BoolParameter* toAdd = new BoolParameter(p.m_Data, p.m_Char, p.m_Optional);
	*(bool*)toAdd->m_Data = false;

	m_Parameters[m_CurrSize] = toAdd;
	m_CurrSize++;
}


#endif
#endif