#pragma once

#include <string>
#include <stdexcept>


///=============================================================
/// \class	KeyException
/// \author	KeyC0de
/// \date	2019/09/11 19:38
/// \brief	Custom exception hierarchy
/// \brief	child/specific exception classes are nested in other classes & specify the concrete exception description
///=============================================================
class KeyException
	: public std::exception
{
	int m_line;
	std::string m_file;
	std::string m_function;
protected:
	mutable std::string m_description;
public:
	KeyException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;
	virtual ~KeyException() noexcept = default;

	virtual const char* what() const noexcept override;
	///===================================================
	// \brief returns the type of the exception - name of child exception class
	// \date 2019/09/11 20:18
	virtual std::string getType() const noexcept;
	inline unsigned getLine() const noexcept;
	inline const std::string& getFile() const noexcept;
	inline const std::string& getFunction() const noexcept;
};


#define THROW_KEY_EXCEPTION( msg ) __debugbreak();\
	throw KeyException( __LINE__, __FILE__, __FUNCTION__, msg );