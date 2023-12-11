#pragma once


class NonCopyable
{
public:
	NonCopyable() = default;
	NonCopyable( const NonCopyable &rhs ) = delete;
	NonCopyable& operator=( const NonCopyable &rhs ) = delete;
	NonCopyable( NonCopyable &rhs ) = delete;
	NonCopyable& operator=( NonCopyable &rhs ) = delete;
	NonCopyable( NonCopyable &&rhs ) = default;
	NonCopyable& operator=( NonCopyable &&rhs ) = default;
protected:
	~NonCopyable() noexcept = default;	// remember an explicitly defaulted dtor suppresses movers
};

class NonCopyableAndNonMovable
{
public:
	NonCopyableAndNonMovable() = default;
	NonCopyableAndNonMovable( const NonCopyableAndNonMovable &rhs ) = delete;
	NonCopyableAndNonMovable& operator=( const NonCopyableAndNonMovable &rhs ) = delete;
	NonCopyableAndNonMovable( NonCopyableAndNonMovable &rhs ) = delete;
	NonCopyableAndNonMovable& operator=( NonCopyableAndNonMovable &rhs ) = delete;
	NonCopyableAndNonMovable( NonCopyableAndNonMovable &&rhs ) = delete;
	NonCopyableAndNonMovable& operator=( NonCopyableAndNonMovable &&rhs ) = delete;
protected:
	~NonCopyableAndNonMovable() noexcept = default;
};

class NonAssignable
{
public:
	NonAssignable() = default;
	NonAssignable( const NonAssignable &rhs ) = default;
	//NonAssignable( NonAssignable &rhs ) = default;	// multiple versions of a defaulted special member are not allowed
	NonAssignable& operator=( const NonAssignable &rhs ) = delete;
	NonAssignable& operator=( NonAssignable &rhs ) = delete;
	NonAssignable( NonAssignable &&rhs ) = default;
	NonAssignable& operator=( NonAssignable &&rhs ) = delete;
protected:
	~NonAssignable() noexcept = default;
};