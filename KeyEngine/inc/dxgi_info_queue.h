#pragma once

#include <vector>
#include <string>
#include <dxgidebug.h>
#include "console.h"
#pragma warning( disable: 4265 )
#	include <wrl.h>
#pragma warning( default: 4265 )


// ref: https://walbourn.github.io/dxgi-debug-device/
// In DirectX 11.1 + you can get helpful information from DX Device Context functions
// using the DXGIGetDebugInterface given we have the debug layer of our device enabled
// The confusing part is that the function you need, DXGIGetDebugInterface is not defined
//		in any header and is not present in anyimport library.
// So we have to load it dynamically
class DxgiInfoQueue final
	: public NonCopyable
{
	static inline const DXGI_DEBUG_ID s_msgProducer = DXGI_DEBUG_ALL;
	size_t m_index = 0u;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_pDxgiInfoQueue;
public:
	DxgiInfoQueue();

	//	\function	markQueueIndex	||	\date	2021/01/14 21:47
	//	\brief	call this on frame end right before Present()ation
	//			it sets the message queue index so that with the next call to getInfo we
	//			will get info messages generated after the last call to markQueueIndex()
	void markQueueIndex() noexcept;
	std::vector<std::string> getInfoMessages();
};


#if defined _DEBUG && !defined NDEBUG
#	define DXGI_GET_QUEUE_INFO( gph ) \
	{\
		KeyConsole &console = KeyConsole::getInstance();\
		const auto &messages = gph.getInfoQueue().getInfoMessages();\
		if ( !messages.empty() )\
		{\
			for ( const auto &msg : messages )\
			{\
				console.log( msg + "\n" );\
			}\
			__debugbreak();\
		}\
		gph.getInfoQueue().markQueueIndex();\
	}
#else
#	define DXGI_GET_QUEUE_INFO (void)0;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define DXGI_GET_QUEUE_INFO_P( gph ) \
	{\
		KeyConsole &console = KeyConsole::getInstance();\
		const auto &messages = gph->getInfoQueue().getInfoMessages();\
		if ( !messages.empty() )\
		{\
			for ( const auto &msg : messages )\
			{\
				console.log( msg + "\n" );\
			}\
			__debugbreak();\
		}\
		gph->getInfoQueue().markQueueIndex();\
	}
#else
#	define DXGI_GET_QUEUE_INFO_P (void)0;
#endif

// this define is only to be used by the graphics class
#if defined _DEBUG && !defined NDEBUG
#	define DXGI_GET_QUEUE_INFO_GFX \
	{\
		KeyConsole &console = KeyConsole::getInstance();\
		const auto &messages = m_infoQueue.getInfoMessages();\
		if ( !messages.empty() )\
		{\
			for ( const auto &msg : messages )\
			{\
				console.log( msg + "\n" );\
			}\
			__debugbreak();\
		}\
		m_infoQueue.markQueueIndex();\
	}
#else
#	define DXGI_GET_QUEUE_INFO_GFX (void)0;
#endif