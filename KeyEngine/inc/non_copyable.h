#pragma once


class NonCopyable
{
public:
	NonCopyable() = default;
	NonCopyable( const NonCopyable& rhs ) = delete;
	NonCopyable& operator=( const NonCopyable& rhs ) = delete;
	NonCopyable( NonCopyable&& rhs ) = default;
	NonCopyable& operator=( NonCopyable&& rhs ) = default;
protected:
	~NonCopyable() noexcept = default;	// remember an explicitly defaulted dtor suppresses movers
};

class NonCopyableAndNonMovable
{
public:
	NonCopyableAndNonMovable() = default;
	NonCopyableAndNonMovable( const NonCopyableAndNonMovable& rhs ) = delete;
	NonCopyableAndNonMovable& operator=( const NonCopyableAndNonMovable& rhs ) = delete;
	NonCopyableAndNonMovable( NonCopyableAndNonMovable&& rhs ) = delete;
	NonCopyableAndNonMovable& operator=( NonCopyableAndNonMovable&& rhs ) = delete;
protected:
	~NonCopyableAndNonMovable() noexcept = default;
};

class NonAssignable
{
public:
	NonAssignable() = default;
	NonAssignable( const NonAssignable& rhs ) = default;
	NonAssignable& operator=( const NonAssignable& rhs ) = delete;
	NonAssignable( NonAssignable&& rhs ) = default;
	NonAssignable& operator=( NonAssignable&& rhs ) = delete;
protected:
	~NonAssignable() noexcept = default;
};