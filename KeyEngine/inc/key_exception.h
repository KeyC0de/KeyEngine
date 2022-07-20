#pragma once

#include <string>
#include <stdexcept>


//=============================================================
// \class	KeyException
//
// \author	KeyC0de
// \date	2019/09/11 19:38
//
// \brief	Custom exception hierarchy
//			Protected ctor
//			child/specific exception classes are nested in other classes & specify
//				the concrete exception description
//=============================================================
class KeyException
	: public std::exception
{
	int m_line;
	std::string m_file;
	std::string m_function;
protected:
	mutable std::string m_description;
protected:
	KeyException( int line, const char* file, const char* function,
		const std::string &msg ) noexcept;
public:
	virtual ~KeyException() noexcept = default;

	virtual const char* what() const noexcept override;
	//===================================================
	// \brief returns the type of the exception - name of child exception class
	// \date 2019/09/11 20:18
	virtual const std::string getType() const noexcept = 0;
	inline const unsigned getLine() const noexcept;
	inline const std::string getFile() const noexcept;
	inline const std::string getFunction() const noexcept;
};