#include "key_sound.h"
#include "winner.h"
#include "assertions_console.h"
#include "utils.h"
#include "os_utils.h"
#include <algorithm>
#if defined _DEBUG && !defined NDEBUG
#	include <iostream>
#endif // _DEBUG
#include "reporter_access.h"
#include "reporter_listener_events.h"
#include "thread_poolj.h"

#pragma comment( lib, "xaudio2_8.lib" )


namespace mwrl = Microsoft::WRL;

namespace wav
{
	// wav properties - all sounds must have the same format
	static constexpr WORD nChannelsPerSound = 2u;
	static constexpr DWORD nSamplesPerSec = 48000u;	// valid: 44100u, 48000u, 96000u
	static constexpr WORD nBitsPerSample = 16u;
}// wav


SoundManager::SoundManager( WAVEFORMATEXTENSIBLE *format )
{
	HRESULT hres;
	hres = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
	ASSERT_HRES_IF_FAILED;

	// keep a pointer to the format
	m_pFormat = format;

	hres = XAudio2Create( &m_pXAudio2, 0u, XAUDIO2_DEFAULT_PROCESSOR );
	ASSERT_HRES_IF_FAILED;

	hres = m_pXAudio2->CreateMasteringVoice( &m_pMasterVoice );
	ASSERT_HRES_IF_FAILED;

	// create the channels
	m_idleChannels.reserve( s_nMaxAudioChannels );
	for ( size_t i = 0; i < s_nMaxAudioChannels; ++i )
	{
		m_idleChannels.emplace_back( std::make_unique<Channel>() );
	}
	m_submixes.reserve( s_nMaxSubmixes );
	for ( size_t i = 0; i < s_nMaxSubmixes; ++i )
	{
		m_submixes.emplace_back( std::make_unique<Submix>() );
	}
}

SoundManager::~SoundManager() noexcept
{
	std::unique_lock<std::mutex> ul{m_mu};
	m_occupiedChannels.clear();
	m_idleChannels.clear();
	m_submixes.clear();
	m_pMasterVoice->DestroyVoice();
	m_pMasterVoice = nullptr;
}

SoundManager::Channel::~Channel() noexcept
{
	if ( m_pSourceVoice || m_pSound )
	{
		m_pSourceVoice->DestroyVoice();
		m_pSourceVoice = nullptr;
		m_pSound = nullptr;
	}
}

SoundManager::Channel::Channel( Channel &&rhs ) cond_noex
	:
	m_pSourceVoice{rhs.m_pSourceVoice},
	m_pSound{rhs.m_pSound}
{
	rhs.m_pSourceVoice = nullptr;
	rhs.m_pSound = nullptr;
}

auto SoundManager::Channel::operator=( Channel &&rhs ) cond_noex -> Channel&
{
	Channel tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

bool SoundManager::Channel::setupChannel( SoundManager &soundManager,
	Sound &sound )
{
	m_pSound = &sound;

	class VoiceCallback
		: public IXAudio2VoiceCallback
	{
	public:
		virtual ~VoiceCallback() noexcept
		{

		}

		// Called when the voice is about to start processing a new audio buffer.
		void STDMETHODCALLTYPE OnBufferStart( void *pBufferContext ) override
		{
			pass_;
		}

		// Called when the voice finishes processing a buffer.
		void STDMETHODCALLTYPE OnBufferEnd( void *pBufferContext ) override
		{
			Channel &channel = *reinterpret_cast<Channel*>( pBufferContext );
			channel.stopSound();
			{
				//std::unique_lock<std::mutex> lg{channel.m_pSound->m_mu};
				// clear Sound's channel if it exists
				channel.m_pSound->m_busyChannels.erase( std::find( channel.m_pSound->m_busyChannels.begin(), channel.m_pSound->m_busyChannels.end(), &channel ) );
				// notify any thread that might be waiting for activeChannels
				// to become zero (i.e. thread calling destructor)
				channel.m_pSound->m_condVar.notify_all();
			}
			SoundManager::getInstance().deactivateChannel( channel );
		}

		// Called when the voice reaches the end position of a loop.
		void STDMETHODCALLTYPE OnLoopEnd( void *pBufferContext ) override
		{
			pass_;
		}

		// Called when the voice has just finished playing a contiguous audio stream.
		void STDMETHODCALLTYPE OnStreamEnd() override
		{
			pass_;
		}

		// Called when a critical error occurs during voice processing.
		void STDMETHODCALLTYPE OnVoiceError( void *pBufferContext,
			HRESULT Error ) override
		{
			pass_;
		}

		// Called just after the processing pass for the voice ends.
		void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override
		{
			pass_;
		}

		// Called during each processing pass for each voice, just
		void STDMETHODCALLTYPE OnVoiceProcessingPassStart( UINT32 bytesRequired ) override
		{
			pass_;
		}
	};

	static VoiceCallback voiceCallback;
	m_pSound->m_pXaudioBuffer->pContext = this;

	HRESULT hres;
	UINT32 sourceVoiceCreationFlags = 0u;

	auto &waveFormat = m_pSound->m_pWaveFormat;

	// 5. optional - specify an output (submix) voice for this source voice
	const auto &soundSubmixName = sound.getSubmixName();
	if ( soundSubmixName != "" )
	{
		//std::unique_lock<std::mutex> lg{soundManager.m_mu, std::try_to_lock}; // already locked!
		// grab a submix from the bag and fill it up
		auto &newSubmix = soundManager.m_submixes.back();
		newSubmix->setName( soundSubmixName );

		soundManager.m_pXAudio2->CreateSubmixVoice( &newSubmix->m_pSubmixVoice, waveFormat->Format.nChannels, waveFormat->Format.nSamplesPerSec, 0u, 0u, nullptr, nullptr );

		//  create the voice sends structure
		newSubmix->m_outputVoiceSendDesc = {0, newSubmix->m_pSubmixVoice};
		newSubmix->m_outputVoiceSends = {1,	&newSubmix->m_outputVoiceSendDesc};

		// 6. Create the source voice
		hres = soundManager.m_pXAudio2->CreateSourceVoice( &m_pSourceVoice, (WAVEFORMATEX*)waveFormat.get(), sourceVoiceCreationFlags, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, &newSubmix->m_outputVoiceSends, nullptr );
	}
	else
	{
		// 6. Create the source voice
		hres = soundManager.m_pXAudio2->CreateSourceVoice( &m_pSourceVoice, (WAVEFORMATEX*)waveFormat.get(), sourceVoiceCreationFlags, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, nullptr, nullptr );
	}
	ASSERT_HRES_IF_FAILED;

	// set steady sample rate
	if ( waveFormat->Format.nSamplesPerSec != wav::nSamplesPerSec )
	{
		waveFormat->Format.nSamplesPerSec = wav::nSamplesPerSec;
		hres = m_pSourceVoice->SetSourceSampleRate( wav::nSamplesPerSec );
		ASSERT_HRES_IF_FAILED;
	}

	// 6. submit the XAUDIO2_BUFFER to the source voice
	hres = m_pSourceVoice->SubmitSourceBuffer( m_pSound->m_pXaudioBuffer.get() );
	ASSERT_HRES_IF_FAILED;

	return true;
}

void SoundManager::Channel::playSound( Sound *sound,
	const float volume )
{
	ASSERT( m_pSound, "Null Sound!" );
	ASSERT( m_pSourceVoice, "Null Voice!" );

	sound->m_busyChannels.emplace_back( this );

	HRESULT hres = m_pSourceVoice->SetVolume( volume );
	ASSERT_HRES_IF_FAILED;
	hres = m_pSourceVoice->Start( 0u );
	ASSERT_HRES_IF_FAILED;
}

void SoundManager::Channel::stopSound() cond_noex
{
	ASSERT( m_pSound, "Sound was not initialized!" );
	ASSERT( m_pSourceVoice, "Voice was not set!" );
	m_pSourceVoice->Stop();
	m_pSourceVoice->FlushSourceBuffers();
}

void SoundManager::Channel::rechannel( const Sound *pOldSound,
	Sound *pNewSound )
{
	ASSERT( pOldSound == pNewSound, "Channel mismatch!" );
	m_pSound = pNewSound;
}

Sound* SoundManager::Channel::getSound() const cond_noex
{
	ASSERT( m_pSound, "Sound is null!" );
	return m_pSound;
}

SoundManager& SoundManager::getInstance( WAVEFORMATEXTENSIBLE *format )
{
	static SoundManager soundManager{format};
	return soundManager;
}

void SoundManager::setMasterVolume( const float volume )
{
	m_pMasterVoice->SetVolume( volume );
}

void SoundManager::playChannelSound( class Sound *sound,
	const float volume )
{
	std::unique_lock<std::mutex> ul{m_mu};
	if ( !m_idleChannels.empty() && m_occupiedChannels.size() < s_nMaxAudioChannels )
	{
		auto &channel = m_idleChannels.back();
		channel->setupChannel( *this, *sound );
		m_occupiedChannels.emplace_back( std::move( channel ) );
		m_idleChannels.pop_back();
		m_occupiedChannels.back()->playSound( sound, volume );
	}
}

void SoundManager::deactivateChannel( Channel &channel )
{
	std::lock_guard<std::mutex> lg{m_mu};
	// convert ptr/ref to container iterator
	auto it = std::find_if( m_occupiedChannels.begin(), m_occupiedChannels.end(),
		[&channel] ( const std::unique_ptr<Channel>& cha )
		{
			return &channel == cha.get();
		} );
	ASSERT( &it != nullptr, "Channel was already absent!" );

	m_idleChannels.emplace_back( std::move( *it ) );
	m_occupiedChannels.erase( it );
}

#pragma region LibrarySoundReading

#ifdef _XBOX	// big endian
#define fourccRIFF	'RIFF'
#define fourccDATA	'data'
#define fourccFMT	'fmt '
#define fourccWAVE	'WAVE'
#define fourccXWMA	'XWMA'
#define fourccDPDS	'dpds'
#else	// little endian
#define fourccRIFF	'FFIR'
#define fourccDATA	'atad'
#define fourccFMT	' tmf'
#define fourccWAVE	'EVAW'
#define fourccXWMA	'AMWX'
#define fourccDPDS	'sdpd'
#endif

HRESULT Sound::findChunk( HANDLE file,
	DWORD fourcc,
	DWORD &chunkSize,
	DWORD &chunkDataPosition )
{
	HRESULT hr = S_OK;
	if ( SetFilePointer( file, 0, nullptr, FILE_BEGIN ) == INVALID_SET_FILE_POINTER )
	{
		return HRESULT_FROM_WIN32( GetLastError() );
	}

	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD RIFFDataSize = 0;
	DWORD fileType;
	DWORD bytesRead = 0;
	DWORD fileOffset = 0;

	while ( hr == S_OK )
	{
		if ( ReadFile( file, &chunkType, sizeof( DWORD ), &bytesRead, nullptr ) == 0 )
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );
		}

		if ( ReadFile( file, &chunkDataSize, sizeof( DWORD ), &bytesRead, nullptr ) == 0 )
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );
		}

		switch ( chunkType )
		{
		case fourccRIFF:
			RIFFDataSize = chunkDataSize;
			chunkDataSize = 4;
			if ( 0 == ReadFile( file, &fileType, sizeof( DWORD ), &bytesRead, nullptr ) )
			{
				hr = HRESULT_FROM_WIN32( GetLastError() );
			}
			break;

		default:
			if ( SetFilePointer( file/*-V303*/, chunkDataSize, nullptr, FILE_CURRENT ) == INVALID_SET_FILE_POINTER )
			{
				return HRESULT_FROM_WIN32( GetLastError() );
			}
		}

		fileOffset += sizeof( DWORD ) * 2;

		if ( chunkType == fourcc )
		{
			chunkSize = chunkDataSize;
			chunkDataPosition = fileOffset;
			return S_OK;
		}

		fileOffset += chunkDataSize;

		if ( bytesRead >= RIFFDataSize )
		{
			return S_FALSE;
		}
	}

	return S_OK;
}

HRESULT Sound::readChunkData( HANDLE file,
	void *buffer,
	DWORD buffersize,
	DWORD bufferoffset )
{
	HRESULT hr = S_OK;
	if ( SetFilePointer( file/*-V303*/, bufferoffset, nullptr, FILE_BEGIN ) == INVALID_SET_FILE_POINTER )
	{
		return HRESULT_FROM_WIN32( GetLastError() );
	}
	DWORD bytesRead;
	if ( ReadFile( file, buffer, buffersize, &bytesRead, nullptr ) == 0 )
	{
		hr = HRESULT_FROM_WIN32( GetLastError() );
	}
	return hr;
}

#pragma endregion

Sound::Sound( const char *zsFilename,
	const std::string &defaultName,
	const std::string &defaultSubmixName )
	:
	m_name{defaultName},
	m_submixName{defaultSubmixName},
	// Initialize wave format and audio buffer
	m_pWaveFormat{std::make_unique<WAVEFORMATEXTENSIBLE>()},
	m_pXaudioBuffer{std::make_unique<XAUDIO2_BUFFER>()}
{
	HANDLE file = CreateFileW( util::s2ws( zsFilename ).data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr );
	if ( file == INVALID_HANDLE_VALUE )
	{
		ASSERT_IF_FAILED( HRESULT_FROM_WIN32( GetLastError() ) );
	}

	HRESULT hres = SetFilePointer( file/*-V303*/, 0, nullptr, FILE_BEGIN );
	if ( (DWORD) hres == INVALID_SET_FILE_POINTER )
	{
		ASSERT_HRES_IF_FAILED( HRESULT_FROM_WIN32( GetLastError() ) );
	}

	// 1. locate the 'RIFF' chunk in the audio file and check the file type
	DWORD chunkSize;
	DWORD chunkPosition;

	hres = findChunk( file, fourccRIFF, chunkSize, chunkPosition );
	ASSERT_HRES_IF_FAILED;

	// check file type should be fourccWAVE = 'XWMA'
	DWORD fileType;
	hres = readChunkData( file, &fileType, sizeof DWORD, chunkPosition );
	ASSERT_HRES_IF_FAILED;
	if ( fileType != fourccWAVE )
	{
		std::cout << "Unsupported Filetype '" << fileType << "' discovered:\n";
#if defined _DEBUG && !defined NDEBUG
		__debugbreak();
#endif // _DEBUG
	}

	// 2. locate the 'fmt' chunk and copy its contents into a WAVEFORMATEXTENSIBLE structure
	hres = findChunk( file, fourccFMT, chunkSize, chunkPosition );
	ASSERT_HRES_IF_FAILED;

	hres = readChunkData( file, m_pWaveFormat.get(), chunkSize, chunkPosition );
	ASSERT_HRES_IF_FAILED;

	ASSERT( m_pWaveFormat->Format.nChannels == wav::nChannelsPerSound, "Wrong amount of channels per sound!" );
	ASSERT( m_pWaveFormat->Format.wFormatTag == WAVE_FORMAT_PCM, "Only XPCM format allowed!" );
	ASSERT( m_pWaveFormat->Format.wBitsPerSample == wav::nBitsPerSample, "Wrong bits per sample!" );
	//ASSERT( m_pWaveFormat->Format.nSamplesPerSec == wav::nSamplesPerSec, "Wrong number of samples per second!" );
	ASSERT( m_pWaveFormat->Format.cbSize == 0, "No extra Format information allowed" );

	m_pWaveFormat->Format.nChannels = wav::nChannelsPerSound;
	m_pWaveFormat->Format.nSamplesPerSec = wav::nSamplesPerSec;
	m_pWaveFormat->Format.wBitsPerSample = wav::nBitsPerSample;
	m_pWaveFormat->Format.nBlockAlign = (wav::nBitsPerSample / 8) * wav::nChannelsPerSound;
	m_pWaveFormat->Format.nAvgBytesPerSec = m_pWaveFormat->Format.nBlockAlign * wav::nSamplesPerSec;
	m_pWaveFormat->Format.cbSize = 0;
	m_pWaveFormat->Format.wFormatTag = WAVE_FORMAT_PCM;

	// 3. locate the 'data' of the chunk and copy its contents into a buffer
	hres = findChunk( file, fourccDATA, chunkSize, chunkPosition );
	ASSERT_HRES_IF_FAILED;

	//std::cout << chunkSize << '\n';
	m_pAudioData = std::make_unique<BYTE[]>( static_cast<std::size_t>( chunkSize ) );
	hres = readChunkData( file, m_pAudioData.get(), chunkSize, chunkPosition );
	ASSERT_HRES_IF_FAILED;

	// 4. populate the XAUDIO2_BUFFER structure
	m_pXaudioBuffer->AudioBytes = chunkSize;	// size of the audio buffer in Bytes
	m_pXaudioBuffer->pAudioData = m_pAudioData.get();	// buffer containing audio data
	m_pXaudioBuffer->Flags = XAUDIO2_END_OF_STREAM;
}

Sound::Sound( Sound &&rhs ) cond_noex
	:
	m_name{std::move( rhs.m_name )},
	m_submixName{std::move( rhs.m_submixName )}
{
	// lock the rhs mutex before we copy/move to guard from
	std::unique_lock<std::mutex> ulr{rhs.m_mu, std::defer_lock};
	std::unique_lock<std::mutex> ull{m_mu, std::defer_lock};
	std::lock( ull, ulr );
	m_pAudioData = std::move( rhs.m_pAudioData );
	m_busyChannels = std::move( rhs.m_busyChannels );
	for ( auto &channel : m_busyChannels )
	{
		channel->rechannel( &rhs, this );
	}
	rhs.m_condVar.notify_all();
}

Sound& Sound::operator=( Sound &&rhs ) cond_noex
{
	Sound tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

Sound::~Sound() noexcept
{
	if ( !m_busyChannels.empty() )
	{
		std::unique_lock<std::mutex> ul{m_mu};
		for ( const auto &i : m_busyChannels )
		{
			i->stopSound();
		}
		// wait for those channels to finish playing
		m_condVar.wait( ul, [this] () { return m_busyChannels.size() == 0; } );
	}
}

float Sound::getDuration() const noexcept
{
	const uint64_t nSamples = ( uint64_t(m_pXaudioBuffer->AudioBytes) * 8 ) / ( uint64_t(m_pWaveFormat->Format.wBitsPerSample) * uint64_t(m_pWaveFormat->Format.nChannels) );
	return (nSamples / float(m_pWaveFormat->Format.nSamplesPerSec)) * 1000ull;
}

const std::string& Sound::getName() const cond_noex
{
	return m_name;
}

const std::string& Sound::getSubmixName() const cond_noex
{
	return m_submixName;
}

void Sound::play( const float volume )
{
	SoundManager::getInstance( m_pWaveFormat.get() ).playChannelSound( this, volume );
}

void Sound::stop()
{
	std::lock_guard<std::mutex> lg{m_mu};
	if ( !m_busyChannels.empty() )
	{
		for ( const auto &channel : m_busyChannels )
		{
			channel->stopSound();
		}
	}
	//if ( m_submixName != "" )
	//{
	//
	//}
}

SoundManager::Submix::Submix( const std::string &name )
	:
	m_name{name},
	m_outputVoiceSendDesc{0},
	m_outputVoiceSends{0}
{

}

SoundManager::Submix::~Submix() noexcept
{
	if ( m_pSubmixVoice )
	{
		m_pSubmixVoice->DestroyVoice();
		m_pSubmixVoice = nullptr;
	}
}

const std::string& SoundManager::Submix::getName() const cond_noex
{
	return m_name;
}

void SoundManager::Submix::setName( const std::string &name ) cond_noex
{
	m_name = name;
}

void SoundManager::setSubmixVolume( const Submix &submix,
	const float volume ) cond_noex
{
	std::lock_guard<std::mutex> lg{m_mu};
	for ( const auto &s : m_submixes )
	{
		if ( s->getName() == submix.getName() )
		{
			s->setVolume( volume );
		}
	}
}

void SoundManager::Submix::setVolume( const float volume ) cond_noex
{
	m_pSubmixVoice->SetVolume( volume );
}

SoundManager::Submix::Submix( Submix &&rhs ) cond_noex
	:
	m_name{std::move( rhs.m_name )},
	m_outputVoiceSendDesc{rhs.m_outputVoiceSendDesc},
	m_outputVoiceSends{rhs.m_outputVoiceSends},
	m_pSubmixVoice{rhs.m_pSubmixVoice}
{
	rhs.m_name = "";
	rhs.m_pSubmixVoice = nullptr;
}

SoundManager::Submix& SoundManager::Submix::operator=( Submix &&rhs ) cond_noex
{
	Submix tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SoundPlayer::SoundPlayer()
	:
	IListener<UISoundEvent>()
{
	auto &reportingNexus = ReportingNexus::getInstance();
	static_cast<const IReporter<UISoundEvent>&>( reportingNexus ).addListener( this );
}

SoundPlayer& SoundPlayer::getInstance()
{
	static SoundPlayer soundPlayer{};
	return soundPlayer;
}

void SoundPlayer::notify( const UISoundEvent &event )
{
	switch ( event.m_soundType )
	{
	case UISoundEvent::Component_Hovered:
	{
		auto &threadPool = ThreadPoolJ::getInstance();

		auto lambda = [event] ( nonstd::stop_token st )
			{
				Sound component_hovered_sound{UISoundEvent::getSoundPath( event.m_soundType ), "component_hovered", "ui"};
				component_hovered_sound.play();
				Sleep( static_cast<DWORD>(component_hovered_sound.getDuration()) );
			};
		threadPool.enqueue( lambda );
	}
	case UISoundEvent::Component_Unhovered:
	{

	}
	}
}
