#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <string>
#include <wrl\client.h>
#include <xaudio2.h>
//#include <xaudio2fx.h>
//#include <xapofx.h>
#include <x3daudio.h>

																						  
//============================================================
//	\class	SoundManager
//
//	\author	KeyC0de
//	\date	2020/10/23 21:33
//
//	\brief	singleton class
//			back-end
//			non-copyable & non-movable
//			stores the Sounds in vector of Channel<Sound>
//			encapsulates a Mastering voice which works with a single wave format
//			thus all Sounds contained in a SoundManager object must have the same format
//=============================================================
class SoundManager final
{
public:
	//============================================================
	//	\class	Channel
	//
	//	\author	KeyC0de
	//	\date	2020/10/25 14:01
	//
	//	\brief	back-end
	//			each Sound sticks to a single Channel
	//			at most m_nMaxAudioChannels can play at a certain time
	//=============================================================
	class Channel final
	{
		friend class Sound;

		struct IXAudio2SourceVoice* m_pSourceVoice = nullptr;
		class Sound* m_pSound;
	public:
		Channel() = default;
		Channel( const Channel& rhs ) = delete;
		Channel& operator=( const Channel& rhs ) = delete;
		~Channel() noexcept;
		Channel( Channel&& rhs ) cond_noex;
		Channel& operator=( Channel&& rhs ) cond_noex;

		bool setupChannel( SoundManager& soundManager, class Sound& sound );
		void playSound( class Sound* sound, float volume );
		void stopSound() cond_noex;
		//===================================================
		//	\function	rechannel
		//	\brief  finds new channel for the existing Sound
		//	\date	2020/10/25 19:18
		void rechannel( const class Sound* pOldSound, class Sound* pNewSound );
		class Sound* getSound() const cond_noex;
	};

	class Submix final
	{
		friend class Channel;

		std::string m_name;
		XAUDIO2_SEND_DESCRIPTOR m_outputVoiceSendDesc;
		XAUDIO2_VOICE_SENDS m_outputVoiceSends;
		struct IXAudio2SubmixVoice* m_pSubmixVoice = nullptr;
	public:
		Submix( const std::string &name = "" );
		~Submix() noexcept;
		Submix( const Submix& rhs ) = delete;
		Submix& operator=( const Submix& rhs ) = delete;
		Submix( Submix&& rhs ) cond_noex;
		Submix& operator=( Submix&& rhs ) cond_noex;
	
		std::string getName() const cond_noex;
		void setName( const std::string &name ) cond_noex;
		void setVolume( float volume = 1.0f ) cond_noex;
	};
private:
	WAVEFORMATEXTENSIBLE* m_pFormat;

	Microsoft::WRL::ComPtr<struct IXAudio2> m_pXAudio2;
	struct IXAudio2MasteringVoice* m_pMasterVoice = nullptr;
	std::mutex m_mu;
	std::vector<std::unique_ptr<Channel>> m_occupiedChannels;
	std::vector<std::unique_ptr<Channel>> m_idleChannels;
	std::vector<std::unique_ptr<Submix>> m_submixes;

	static inline constexpr size_t m_nMaxAudioChannels = 16u;
	static inline constexpr size_t m_nMaxSubmixes = 8u;
public:
	//===================================================
	//	\function	getInstance
	//	\brief  return the single instance of the class
	//	\date	2020/10/24 1:48
	static SoundManager& getInstance( WAVEFORMATEXTENSIBLE* format = nullptr );
public:
	SoundManager( const SoundManager& rhs ) = delete;
	SoundManager& operator=( const SoundManager& rhs ) = delete;
	SoundManager( SoundManager&& rhs ) = delete;
	SoundManager& operator=( SoundManager&& rhs ) = delete;
	~SoundManager() noexcept;

	void setMasterVolume( float volume = 1.0f );
	void setSubmixVolume( const Submix& submix, float volume = 1.0f ) cond_noex;
	void playChannelSound( class Sound* sound, float volume );
	//===================================================
	//	\function	deactivateChannel
	//	\brief  removes occupied Channel & places it in the idle list
	//	\date	2020/10/25 19:45
	void deactivateChannel( Channel& channel );
	//void disableSubmixVoice( const Submix& submix );
private:
	SoundManager( WAVEFORMATEXTENSIBLE* format );
};


//============================================================
//	\class	Sound
//
//	\author	KeyC0de
//	\date	2020/10/24 1:51
//
//	\brief	move only
//			front-end
//			encapsulates a sound
//			ctor creates the sound properties
//=============================================================
class Sound final
{
	friend class SoundManager::Channel;

	std::string m_name;
	std::string m_submixName;
	std::unique_ptr<BYTE[]> m_pAudioData;
	std::unique_ptr<WAVEFORMATEXTENSIBLE> m_pWaveFormat;
	std::unique_ptr<struct XAUDIO2_BUFFER> m_pXaudioBuffer;
	std::mutex m_mu;
	std::condition_variable m_condVar;
	std::vector<SoundManager::Channel*> m_busyChannels;	// those are currently playing
public:
	//===================================================
	//	\function	findChunk
	//	\brief  locates chunks in RIFF files
	//	\date	2020/10/25 15:09
	HRESULT findChunk( HANDLE file, DWORD fourcc, DWORD& chunkSize,
		DWORD& chunkDataPosition );
	//===================================================
	//	\function	readChunkData
	//	\brief  read chunk's data (after the chunk has been located)
	//	\date	2020/10/21 17:37
	HRESULT readChunkData( HANDLE file, void* buffer, DWORD buffersize,
		DWORD bufferoffset );
public:
	// TODO: Sound Looping
	//===================================================
	//	\function	Sound
	//	\brief  constructor loads sound file and configures all its properties
	//	\date	2020/10/25 15:04
	Sound( const char* zsFilename, const std::string &name = "",
		const std::string &submixName = "" );
	Sound( const Sound& rhs ) = delete;
	Sound& operator=( const Sound& rhs ) = delete;

	Sound( Sound&& rhs ) cond_noex;
	Sound& operator=( Sound&& rhs ) cond_noex;
	~Sound() noexcept;

	std::string getName() const cond_noex;
	//===================================================
	//	\function	getTypeName
	//	\brief  get sound type eg effects, music, dialogue etc
	//			each sound type corresponds to a Submix voice
	//	\date	2020/10/25 14:05
	std::string getSubmixName() const cond_noex;
	//===================================================
	//	\function	play
	//	\brief  instructs the sound manager to play the sound on free channel(s)
	//	\date	2020/10/25 13:05
	void play( float volume = 1.0f );
	void stop();
};