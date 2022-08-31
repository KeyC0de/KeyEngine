#include "dxgi_info_queue.h"
#include "winner.h"
#include "os_utils.h"

#pragma comment( lib, "dxguid.lib" )


DxgiInfoQueue::DxgiInfoQueue()
{
	using LPDXGIGETDEBUGINTERFACE = HRESULT (WINAPI *)(REFIID, void ** );
	LPDXGIGETDEBUGINTERFACE dxgiGetDebugInterface;

	const HMODULE dxgidebugLib = LoadLibraryExW( L"dxgidebug.dll",
		nullptr,
		LOAD_LIBRARY_SEARCH_SYSTEM32 );

	if ( !dxgidebugLib )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Could not acquire dxgidebug library."s );
#endif
		std::terminate();
	}

	dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>( reinterpret_cast<void*>( GetProcAddress( dxgidebugLib,
		"DXGIGetDebugInterface" ) ) );
	if ( !dxgiGetDebugInterface )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "DXGIGetDebugInterface function was not located in the library."s );
#endif
		std::terminate();
	}

	HRESULT hres = dxgiGetDebugInterface( IID_PPV_ARGS( &m_pDxgiInfoQueue ) );
	ASSERT_HRES_IF_FAILED;

	// determine when to break execution
	m_pDxgiInfoQueue->SetBreakOnSeverity( m_msgProducer,
		DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR,
		true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( m_msgProducer,
		DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION,
		true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( m_msgProducer,
		DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING,
		true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( m_msgProducer,
		DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE,
		true );
	m_pDxgiInfoQueue->SetBreakOnSeverity( m_msgProducer,
		DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO,
		true );
}

void DxgiInfoQueue::markQueueIndex() noexcept
{
	m_index = m_pDxgiInfoQueue->GetNumStoredMessages( m_msgProducer );
}

std::vector<std::string> DxgiInfoQueue::getInfoMessages()
{
	std::vector<std::string> messageDescriptions;
	const size_t lastIndex = m_pDxgiInfoQueue->GetNumStoredMessages( m_msgProducer );
	for( size_t i = m_index; i < lastIndex; ++i )
	{
		HRESULT hres;
		//DXGI_INFO_QUEUE_MESSAGE *pMsg = nullptr;
		size_t messageLengthInBytes = 0;
		// first get the message length
		hres = m_pDxgiInfoQueue->GetMessageW( m_msgProducer,
			i,
			nullptr,
			&messageLengthInBytes );
		ASSERT_HRES_IF_FAILED;
		// allocate storage for the message
		auto pMsgStorage = std::make_unique<wchar_t[]>( messageLengthInBytes );
		auto pMsg = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>( pMsgStorage.get() );
		// create the message
		hres = m_pDxgiInfoQueue->GetMessageW( m_msgProducer,
			i,
			pMsg,
			&messageLengthInBytes );

		messageDescriptions.emplace_back( pMsg->pDescription );
	}
	//m_pDxgiInfoQueue->ClearStoredMessages( m_msgProducer );
	//m_i = 0u;
	return messageDescriptions;
}