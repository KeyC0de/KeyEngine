#pragma once

#include <vector>
#include <string>
#include <dxgidebug.h>
#include <algorithm>
#include "console.h"
#include "key_wrl.h"


///=============================================================
/// \class	DxgiInfoQueue
/// \author	KeyC0de
/// \date	2021/01/14 13:46
/// \brief	ref: https://walbourn.github.io/dxgi-debug-device/
/// \brief	In DirectX 11.1+ you can get helpful information from DX Device Context using the DXGIGetDebugInterface given we have the device debug layer enabled
/// \brief	The confusing part is that the function you need, DXGIGetDebugInterface is not defined in any header and is not present in any import library.
/// \brief	So we have to load it dynamically
///=============================================================
class DxgiInfoQueue final
	: public NonCopyable
{
	static inline const DXGI_DEBUG_ID s_msgProducer = DXGI_DEBUG_ALL;
	size_t m_index = 0u;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_pDxgiInfoQueue;
public:
	DxgiInfoQueue();

	/// \brief	call this on frame end right before Present()ation
	/// \brief	it sets the message queue index so that with the next call to getInfo we
	/// \brief	will get info messages generated after the last call to markQueueIndex()
	void markQueueIndex() noexcept;
	std::vector<std::string> getInfoMessages();
};


#if defined _DEBUG && !defined NDEBUG
#	define DXGI_GET_QUEUE_INFO( gfx ) \
	{\
		KeyConsole &console = KeyConsole::getInstance();\
		const auto &messages = gfx.getInfoQueue().getInfoMessages();\
		if ( !messages.empty() )\
		{\
			const std::string ignored_warnings[] =\
			{\
				"ID3D11DeviceContext::DrawIndexed: The size of the Constant Buffer at slot 2 of the Vertex Shader unit is too small",\
				"ID3D11DeviceContext::DrawIndexed: The size of the Constant Buffer at slot 2 of the Pixel Shader unit is too small"\
			};\
			bool found_non_ignored_warnings = std::none_of( messages.begin(), messages.end(), [&ignored_warnings](const std::string& message)\
				{\
					for ( const std::string &ignored_warning : ignored_warnings )\
					{\
						if ( message.find(ignored_warning) != std::string::npos )\
						{\
							return true;\
						}\
					}\
					return false;\
				} );\
			if ( found_non_ignored_warnings )\
			{\
				for ( const auto &msg : messages )\
				{\
					console.log( msg + "\n" );\
				}\
				__debugbreak();\
			}\
		}\
		gfx.getInfoQueue().markQueueIndex();\
	}
#else
#	define DXGI_GET_QUEUE_INFO (void)0;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define DXGI_GET_QUEUE_INFO_P( gfx ) \
	{\
		KeyConsole &console = KeyConsole::getInstance();\
		const auto &messages = gfx->getInfoQueue().getInfoMessages();\
		if ( !messages.empty() )\
		{\
			const std::string ignored_warnings[] =\
			{\
				"ID3D11DeviceContext::DrawIndexed: The size of the Constant Buffer at slot 2 of the Vertex Shader unit is too small",\
				"ID3D11DeviceContext::DrawIndexed: The size of the Constant Buffer at slot 2 of the Pixel Shader unit is too small"\
			};\
			bool found_non_ignored_warnings = std::none_of( messages.begin(), messages.end(), [&ignored_warnings](const std::string& message)\
				{\
					for ( const std::string &ignored_warning : ignored_warnings )\
					{\
						if ( message.find(ignored_warning) != std::string::npos )\
						{\
							return true;\
						}\
					}\
					return false;\
				} );\
			if ( found_non_ignored_warnings )\
			{\
				for ( const auto &msg : messages )\
				{\
					console.log( msg + "\n" );\
				}\
			}\
			__debugbreak();\
		}\
		gfx->getInfoQueue().markQueueIndex();\
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
			const std::string ignored_warnings[] =\
			{\
				"ID3D11DeviceContext::DrawIndexed: The size of the Constant Buffer at slot 2 of the Vertex Shader unit is too small",\
				"ID3D11DeviceContext::DrawIndexed: The size of the Constant Buffer at slot 2 of the Pixel Shader unit is too small"\
			};\
			bool found_non_ignored_warnings = std::none_of( messages.begin(), messages.end(), [&ignored_warnings](const std::string& message)\
				{\
					for ( const std::string &ignored_warning : ignored_warnings )\
					{\
						if ( message.find(ignored_warning) != std::string::npos )\
						{\
							return true;\
						}\
					}\
					return false;\
				} );\
			if ( found_non_ignored_warnings )\
			{\
				for ( const auto &msg : messages )\
				{\
					console.log( msg + "\n" );\
				}\
				__debugbreak();\
			}\
		}\
		m_infoQueue.markQueueIndex();\
	}
#else
#	define DXGI_GET_QUEUE_INFO_GFX (void)0;
#endif