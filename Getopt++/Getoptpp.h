#ifndef GETOPTPP_H
#define GETOPTPP_H

#ifdef GETOPTPP_PRINT_OUTPUT
#include <iostream>
#endif

#include <sstream>
#include <stdint.h>
#include <stddef.h>
#include <functional>

#define STR_PARENT		Parameter(data, 0, c, optional, validator)
#define NUM_PARENT		Parameter(data, sizeof(T), c, optional, validator)

#define GETOPTPP_ERR_NO_ERROR					0
#define GETOPTPP_ERR_TOO_MANY_ARGUMENTS			-1
#define GETOPTPP_ERR_UNSPECIFIED_NON_OPTIONAL	-2
#define GETOPTPP_ERR_ASKED_USAGE				-3
#define GETOPTPP_ERR_UNKNOWN_OPTION				-4

#define GETOPTPP_ERR_PARSE_ERROR				-5
#define GETOPTPP_ERR_INVALID_VALUE				-6

typedef int GetoptParseError;


#ifndef _WIN32
#include <unistd.h>
#else
int opterr = 1, optind = 1, optopt, optreset;
const char* optarg;
int getopt(int nargc, char* const nargv[], const char* ostr);
#endif

class Parameter
{
	friend class Getoptpp;

public:
	Parameter() = default;
	Parameter(void* data, size_t size, char c, bool optional, std::function<bool(void*)> validator = {}) :
		m_Data(data), m_DataSize(size), m_Char(c), m_Optional(optional), m_Validator(validator) {}

	virtual GetoptParseError Parse(void* data) { return GETOPTPP_ERR_NO_ERROR; }

protected:
	char m_Char;
	bool m_Optional;

	void* m_Data;
	size_t m_DataSize;
	std::function<bool(void*)> m_Validator;
};

class StringParameter : public Parameter
{
public:
	StringParameter() = default;
	StringParameter(void* data, char c, bool optional, std::function<bool(void*)> validator = {});

	GetoptParseError Parse(void* data) override;

private:
	size_t m_MinLength;
	size_t m_MaxLength;

	std::string* m_Pool;
	uint32_t m_PoolSize;

};

template <typename T>
class NumberParameter : public Parameter
{
public:
	NumberParameter() = default;
	NumberParameter(void* data, char c, bool optional, std::function<bool(void*)> validator = {});
	
	GetoptParseError Parse(void* data) override;

private:
	T m_Min;
	T m_Max;
};

class BoolParameter : public Parameter
{
	friend class Getoptpp;
public:
	BoolParameter() = default;
	BoolParameter(void* data, char c);

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
	STR_PARENT {}

/************************************************************ NUMBER PARAMETER ***********************************************************/

template<typename T>
NumberParameter<T>::NumberParameter(void* data, char c, bool optional, std::function<bool(void*)> validator) :
	NUM_PARENT {}

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

BoolParameter::BoolParameter(void* data, char c) : Parameter(data, sizeof(bool), c, true, {}) {}

GetoptParseError BoolParameter::Parse(void* data) 
{
	*(bool*)m_Data = true;
	return GETOPTPP_ERR_NO_ERROR; 
}

/************************************************************ GETOPTPP ***********************************************************/


Getoptpp::Getoptpp(uint32_t nOptions, const std::string& helpMessage) : m_HelpMessage(helpMessage)
{
	m_Parameters = new Parameter*[nOptions];
}

Getoptpp::~Getoptpp()
{
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
		if (!m_Parameters[i]->m_Optional)
		{
			optStr += ":";
			notOptional += m_Parameters[i]->m_Char;
		}
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
	NumberParameter<T>* toAdd = new NumberParameter<T>();
	memcpy(toAdd, &p, sizeof(p));

	m_Parameters[m_CurrSize] = toAdd;
	m_CurrSize++;
}

void Getoptpp::AddStringParam(StringParameter p)
{
	StringParameter* toAdd = new StringParameter();
	memcpy(toAdd, &p, sizeof(p));

	m_Parameters[m_CurrSize] = toAdd;
	m_CurrSize++;
}

void Getoptpp::AddBoolParam(BoolParameter p)
{
	BoolParameter* toAdd = new BoolParameter();
	memcpy(toAdd, &p, sizeof(p));
	*(bool*)toAdd->m_Data = false;

	m_Parameters[m_CurrSize] = toAdd;
	m_CurrSize++;
}


#endif
#endif