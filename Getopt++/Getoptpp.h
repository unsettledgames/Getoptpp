#ifndef GETOPTPP_H
#define GETOPTPP_H

#include <iostream>
#include <sstream>
#include <limits>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <functional>
#include <regex>

#define STR_PARENT		Parameter(data, size, c, optional, validator)
#define NUM_PARENT		Parameter(data, sizeof(T), c, optional, validator)


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

	virtual void Parse(void* data) {}

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
	StringParameter(void* data, size_t size, char c, bool optional, std::function<bool(void*)> validator = {});

	void Parse(void* data) override;

private:
	size_t m_MinLength;
	size_t m_MaxLength;

	std::string* m_Pool;
	uint32_t m_PoolSize;

	std::regex m_Regex;
	std::string m_RegexPattern;

};

template <typename T>
class NumberParameter : public Parameter
{
public:
	NumberParameter() = default;
	NumberParameter(void* data, char c, bool optional, std::function<bool(void*)> validator = {});
	
	void Parse(void* data) override;

private:
	T m_Min;
	T m_Max;
};

typedef NumberParameter<short> ShortParam;
typedef NumberParameter<int> IntParam;
typedef NumberParameter<long> LongParam;

typedef NumberParameter<float> FloatParam;
typedef NumberParameter<double> DoubleParam;

class Getoptpp
{
public:
	Getoptpp(uint32_t nOptions, const std::string& helpMessage = "");
	~Getoptpp();

	void Parse(int argc, char** argv);

	void AddStringParam(StringParameter p);
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

		// found "--"
		if (place[1] && *++place == '-') 
		{ 
			++optind;
			place = "";
			return -1;
		}
	}                                       

	// option letter okay?
	if ((optopt = (int)*place++) == (int)':' || !(oli = strchr(ostr, optopt))) 
	{
		// if the user didn't specify '-' as an option,  assume it means -1.
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++optind;
		if (opterr && *ostr != ':')
			std::cout << "illegal option -- " << optopt << "\n";
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
			if (opterr)
				std::cout << "option requires an argument -- " << optopt << "\n";
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

void StringParameter::Parse(void* data)
{
	std::string param = std::string((char*)data);
	*(std::string*)m_Data = param;

	if (m_Validator)
		m_Validator(m_Data);
}

StringParameter::StringParameter(void* data, size_t size, char c, bool optional, std::function<bool(void*)> validator) :
	STR_PARENT {}

/************************************************************ NUMBER PARAMETER ***********************************************************/

template<typename T>
NumberParameter<T>::NumberParameter(void* data, char c, bool optional, std::function<bool(void*)> validator) :
	NUM_PARENT {}

template <typename T>
void NumberParameter<T>::Parse(void* data)
{
	T tmp;
	std::istringstream iss((char*)data);
	iss >> tmp;

	memcpy(m_Data, &tmp, m_DataSize);
	if (m_Validator)
		m_Validator(m_Data);
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

void Getoptpp::Parse(int argc, char** argv)
{
	std::string optStr = "";
	std::string notOptional = "";

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
			std::cout << m_HelpMessage << std::endl;

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
			std::cerr << "Unknown option: " << (char)c << std::endl;
			std::cout << m_HelpMessage << std::endl;
		}
	}

	if (optind != argc - 1) 
	{
		bool allFound = true;
		for (uint32_t i = 0; i < notOptional.length() && allFound; i++)
		{
			if (notOptional[i] != '?')
			{
				allFound = false;
				std::cout << "Non-optional argument -" << notOptional[i] << " not specified" << std::endl;
			}
		}

		if (allFound)
		{
#ifdef _WIN32
			std::cerr << "Too many arguments or argument before other options\n";
#else
			std::cerr << "Too many arguments\n";
#endif
			std::cout << m_HelpMessage << std::endl;
		}
	}
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


#endif
#endif