#pragma once

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>

class exception_base : public std::exception 
{
protected:
	exception_base(const std::string& msg) : m_msg(msg) {}

	std::string m_msg;

public:
    virtual const char* what() const noexcept override
    {
        return m_msg.c_str();
    }

private:
	static std::ostringstream cnvt;
};

class unrecoverable_except : public exception_base
{
public:
	unrecoverable_except(const std::string& msg) :
		exception_base(msg)
	{	
	}
};
