#include "dxgi_info_queue.h"
#include "winner.h"
#include "os_utils.h"

#pragma comment( lib, "dxguid.lib" )


DxgiInfoQueue::DxgiInfoQueue()
{
	const HMODULE dxgidebugLib = LoadLibraryExW( L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 );

	if ( !dxgidebugLib )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Could not acquire dxgidebug library."s );
#endif
		std::terminate();
	}

	using DXGIGetDebugInterface = HRESULT( WINAPI * )( REFIID, void ** );
	const DXGIGetDebugInterface dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>( reinterpret_cast<void*>( GetProcAddress( dxgidebugLib, "DXGIGetDebugInterface" ) ) );
	if ( !dxgiGetDebugInterface )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "DXGIGetDebugInterface function was not located in the library."s );
#endif
		std::terminate();
	}

	//HRESULT hres = dxgiGetDebugInterface( IID_PPV_ARGS( &m_pDxgiInfoQueue ) );
	HRESULT hres = dxgiGetDebugInterface( __uuidof( IDXGIInfoQueue ), &m_pDxgiInfoQueue );
	ASSERT_HRES_IF_FAILED;

	// determine when to break execution
	m_pDxgiInfoQueue->SetBreakOnSeverity( s_msgProducer, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( s_msgProducer, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( s_msgProducer, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( s_msgProducer, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE, true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( s_msgProducer, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO, true );
}

void DxgiInfoQueue::markQueueIndex() noexcept
{
	m_index = m_pDxgiInfoQueue->GetNumStoredMessages( s_msgProducer );
}

std::vector<std::string> DxgiInfoQueue::getInfoMessages()
{
	std::vector<std::string> messageDescriptions;
	const size_t lastIndex = m_pDxgiInfoQueue->GetNumStoredMessages( s_msgProducer );
	for( size_t i = m_index; i < lastIndex; ++i )
	{
		HRESULT hres;
		size_t messageLengthInBytes = 0;

		// first get the message length
		hres = m_pDxgiInfoQueue->GetMessageW( s_msgProducer, i, nullptr, &messageLengthInBytes );
		ASSERT_HRES_IF_FAILED;

		// allocate storage for the message
		auto pMsgStorage = std::make_unique<wchar_t[]>( messageLengthInBytes );
		DXGI_INFO_QUEUE_MESSAGE* pMsg = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>( pMsgStorage.get() );

		// create the message
		hres = m_pDxgiInfoQueue->GetMessageW( s_msgProducer, i, pMsg, &messageLengthInBytes );
		ASSERT_HRES_IF_FAILED;

		// add it to the list of messages
		messageDescriptions.emplace_back( pMsg->pDescription );
	}
	//m_pDxgiInfoQueue->ClearStoredMessages( s_msgProducer );
	//m_i = 0u;
	return messageDescriptions;
}