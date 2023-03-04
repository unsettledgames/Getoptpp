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

#define STR_MIN_MAX		m_MinLength(0), m_MaxLength(ULLONG_MAX)
#define NUM_MIN_MAX		m_Min(std::numeric_limits<T>().max()), m_Max(std::numeric_limits<T>().min())

#define LONG_MIN_MAX	m_Min(LONG_MIN), m_Max(LONG_MAX)
#define REAL_MIN_MAX	m_Min(DOUBLE_MIN), m_Max(DOUBLE_MAX)

#define STR_POOL		m_Pool(nullptr), m_PoolSize(0)
#define STR_REGEX		m_Regex("(.*?)"), m_RegexPattern("(.*?)")


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

	virtual bool Validate(void* data) { return true; }
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
	StringParameter(void* data, size_t size, char c, bool optional, std::string regex, std::function<bool(void*)> validator = {});
	StringParameter(void* data, size_t size, char c, bool optional, std::string* pool, size_t poolSize, std::function<bool(void*)> validator = {});
	StringParameter(void* data, size_t size, char c, bool optional, size_t minLength, size_t maxLength, std::function<bool(void*)> validator = {});

	bool Validate(void* data) override;
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
	NumberParameter(void* data, char c, bool optional, T min, T max, std::function<bool(void*)> = {});
	
	bool Validate(void* data) override;
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
	if (Validate(&param))
		*(std::string*)m_Data = param;
}

bool StringParameter::Validate(void* data)
{
	std::string str = *(std::string*)data;

	// Length constraints
	if (str.length() < m_MinLength || str.length() > m_MaxLength)
		return false;
	
	// Pool constraints
	bool inPool = m_PoolSize == 0;
	for (uint32_t i = 0; i < m_PoolSize && !inPool; i++)
		if (m_Pool[i] == str)
			inPool = true;
	if (!inPool)
		return inPool;

	// Regex constraint
	m_Regex.assign(m_RegexPattern);
	if (!std::regex_match(str.c_str(), m_Regex))
		return false;

	if (m_Validator)
		return m_Validator(data);
	return true;
}

StringParameter::StringParameter(void* data, size_t size, char c, bool optional, std::function<bool(void*)> validator) :
	STR_PARENT, STR_MIN_MAX, STR_POOL, STR_REGEX {}

StringParameter::StringParameter(void* data, size_t size, char c, bool optional, std::string regex,
	std::function<bool(void*)> validator) : STR_PARENT, STR_MIN_MAX, STR_POOL, m_Regex(std::regex(regex)), m_RegexPattern(regex){}

StringParameter::StringParameter(void* data, size_t size, char c, bool optional, std::string* pool, size_t poolSize,
	std::function<bool(void*)> validator) : STR_PARENT, STR_MIN_MAX, m_Pool(pool), m_PoolSize(poolSize), STR_REGEX{}

StringParameter::StringParameter(void* data, size_t size, char c, bool optional, size_t minLength, size_t maxLength, 
	std::function<bool(void*)> validator) : STR_PARENT, m_MinLength(minLength), m_MaxLength(maxLength), STR_POOL, STR_REGEX{}

/************************************************************ NUMBER PARAMETER ***********************************************************/

template<typename T>
NumberParameter<T>::NumberParameter(void* data, char c, bool optional, std::function<bool(void*)> validator) :
	NUM_PARENT, NUM_MIN_MAX {}

template<typename T>
NumberParameter<T>::NumberParameter(void* data, char c, bool optional, T min, T max, std::function<bool(void*)> validator) :
	NUM_PARENT, m_Min(min), m_Max(max) {}

template <typename T>
void NumberParameter<T>::Parse(void* data)
{
	T tmp;
	std::istringstream iss((char*)data);
	iss >> tmp;

	memcpy(m_Data, &tmp, m_DataSize);
}

template <typename T>
bool NumberParameter<T>::Validate(void* data)
{
	T param = *(T*)data;
	if (param < m_Min || param > m_Max)
		return false;

	if (m_Validator)
		return m_Validator(data);
	return true;
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
	std::string optStr;
	for (uint32_t i = 0; i < m_CurrSize; i++)
	{
		optStr += m_Parameters[i]->m_Char;
		if (!m_Parameters[i]->m_Optional)
			optStr += ":";
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
#ifdef _WIN32
		std::cerr << "Too many arguments or argument before other options\n";
#else
		std::cerr << "Too many arguments\n";
#endif
		std::cout << m_HelpMessage << std::endl;
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