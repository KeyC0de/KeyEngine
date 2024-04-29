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
#include "non_copyable.h"


//============================================================
//	\class	SoundManager
//	\author	KeyC0de
//	\date	2020/10/23 21:33
//	\brief	singleton class
//			back-end
//			non-copyable & non-movable
//			stores the Sounds in vector of Channel<Sound>
//			encapsulates a Mastering voice which works with a single wave format
//			thus all Sounds contained in a SoundManager object must have the same format
//=============================================================
class SoundManager final
	: public NonCopyableAndNonMovable
{
public:
	//============================================================
	//	\class	Channel
	//	\author	KeyC0de
	//	\date	2020/10/25 14:01
	//	\brief	back-end
	//			each Sound sticks to a single Channel
	//			at most s_nMaxAudioChannels can play at a certain time
	//=============================================================
	class Channel final
		: public NonCopyable
	{
		friend class Sound;

		struct IXAudio2SourceVoice *m_pSourceVoice = nullptr;
		Sound *m_pSound;
	public:
		Channel() = default;
		Channel( Channel &&rhs ) cond_noex;
		Channel& operator=( Channel &&rhs ) cond_noex;
		~Channel() noexcept;

		bool setupChannel( SoundManager &soundManager, Sound &sound );
		void playSound( Sound *sound, const float volume );
		void stopSound() cond_noex;
		//	\function	rechannel	||	\date	2020/10/25 19:18
		//	\brief  finds new channel for the existing Sound
		void rechannel( const Sound *pOldSound, Sound *pNewSound );
		Sound* getSound() const cond_noex;
	};

	class Submix final
		: public NonCopyable
	{
		friend class Channel;

		std::string m_name;
		XAUDIO2_SEND_DESCRIPTOR m_outputVoiceSendDesc;
		XAUDIO2_VOICE_SENDS m_outputVoiceSends;
		struct IXAudio2SubmixVoice *m_pSubmixVoice = nullptr;
	public:
		Submix( const std::string &name = "" );
		~Submix() noexcept;
		Submix( Submix &&rhs ) cond_noex;
		Submix& operator=( Submix &&rhs ) cond_noex;

		const std::string& getName() const cond_noex;
		void setName( const std::string &name ) cond_noex;
		void setVolume( const float volume = 1.0f ) cond_noex;
	};
private:
	WAVEFORMATEXTENSIBLE *m_pFormat;
	Microsoft::WRL::ComPtr<struct IXAudio2> m_pXAudio2;
	struct IXAudio2MasteringVoice *m_pMasterVoice = nullptr;
	std::mutex m_mu;
	std::vector<std::unique_ptr<Channel>> m_occupiedChannels;
	std::vector<std::unique_ptr<Channel>> m_idleChannels;
	std::vector<std::unique_ptr<Submix>> m_submixes;

	static inline constexpr size_t s_nMaxAudioChannels = 16u;
	static inline constexpr size_t s_nMaxSubmixes = 8u;
public:
	//	\function	getInstance	||	\date	2020/10/25 21:38
	//	\brief  return the single instance of the class
	static SoundManager& getInstance( WAVEFORMATEXTENSIBLE *format = nullptr );
public:
	~SoundManager() noexcept;

	void setMasterVolume( const float volume = 1.0f );
	void setSubmixVolume( const Submix &submix, const float volume = 1.0f ) cond_noex;
	void playChannelSound( class Sound *sound, const float volume );
	//	\function	deactivateChannel	||	\date	2020/10/25 20:18
	//	\brief  removes occupied Channel & places it in the idle list
	void deactivateChannel( Channel &channel );
	//void disableSubmixVoice( const Submix &submix );
private:
	SoundManager( WAVEFORMATEXTENSIBLE *format );
};


//============================================================
//	\class	Sound
//	\author	KeyC0de
//	\date	2020/10/24 1:51
//	\brief	move only
//			front-end
//			encapsulates a sound
//			ctor creates the sound properties
//=============================================================
class Sound final
	: public NonCopyable
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
	//	\function	findChunk	||	\date	2020/10/25 15:09
	//	\brief	locates chunks in RIFF files
	HRESULT findChunk( HANDLE file, DWORD fourcc, DWORD &chunkSize, DWORD &chunkDataPosition );
	//	\function	readChunkData
	//	\brief  read chunk's data (after the chunk has been located)

	HRESULT readChunkData( HANDLE file, void *buffer, DWORD buffersize, DWORD bufferoffset );
public:
	// #TODO: Sound Looping
	//	\function	Sound	||	\date	2020/10/25 15:04
	//	\brief	constructor loads sound file and configures all its properties
	Sound( const char *zsFilename, const std::string &name = "", const std::string &submixName = "" );
	Sound( Sound &&rhs ) cond_noex;
	Sound& operator=( Sound &&rhs ) cond_noex;
	~Sound() noexcept;

	const std::string& getName() const cond_noex;
	//	\function	getTypeName	\date	2020/10/25 14:05
	//	\brief	get sound type eg effects, music, dialogue etc. --- each sound type corresponds to a Submix voice
	const std::string& getSubmixName() const cond_noex;
	//	\function	play	||	\date	2020/10/25 13:05
	//	\brief	instructs the sound manager to play the sound on free channel(s)
	void play( const float volume = 1.0f );
	void stop();
};