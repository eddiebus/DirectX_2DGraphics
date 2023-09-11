#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <ResourceLoading.h>
//Xaudio 2
#pragma comment(lib,"Xaudio2.lib")
#include <xaudio2.h>
//CComPtr
#include <atlbase.h>


namespace EGame
{
	namespace Audio
	{
		//Class for Wave filetpye
		class Wave
		{
		public:
			Wave() 
			{
				refCount = new int;
				*refCount = -1;
			}
			//Copy Constructer
			Wave(Wave& otherWave) 
			{
				operator=(otherWave);
			}
			//---
			Wave(EResource inputResource)
			{
				if (!inputResource) return;

				char* fileP = (char*)inputResource.GetDataPointer();
				size_t fileByteCount = inputResource.GetDataSize();

				char* riffP = nullptr;
				std::wstring riffTypeName;
				char* formatP = nullptr;
				char* dataP = nullptr;
				UINT dataSize = 0;

				//Find "Riff" in the file contents
				for (int i = 0; i < fileByteCount - 4; i++)
				{
					std::wstring riff;
					for (int c = 0; c < 4; c++)
						riff += (char)*(fileP + c);
					
					if (riff == L"RIFF")
					{
						riffP = fileP + i;
						OutputDebugString(L"\nFound Riff!\n");
						break;
					}
				}

				//Finding the riff Type (WAVE / XMNA)
				char* riffTypeP = riffP += 8; //The data type is 2 bytes ahead
				for (int i = 0; i < 4; i++)
					riffTypeName += *(riffTypeP + i);

				//Dont see any valid type of WAVE.
				if (riffTypeName != L"WAVE" && riffTypeName != L"XMNA")
					return;
				else
					riffTypeName.clear();
				
				//Look for format chunk
				for (int i = 0; i < fileByteCount - 4; i++)
				{
					std::wstring fmtName;
					for (int c = 0; c < 3; c++)
						fmtName += (char)*(fileP + c + i);

					if (fmtName == L"fmt")
					{
						formatP = fileP + i;
						OutputDebugString(L"\nFound Format!\n");
						break;
					}
				}

				UINT formatDataSize = 0;
				memcpy(&formatDataSize, formatP + 4, sizeof(byte) * 4);

				//Look for data chunk
				for (int i = 0; i < fileByteCount - 4; i++)
				{
					std::wstring dataName;
					for (int c = 0; c < 4; c++)
						dataName += (char)*(fileP + c+i);

					if (dataName == L"data")
					{
						dataP = fileP + i;
						OutputDebugString(L"\nFound Data!\n");

						//Get the size of wave information
						byte* dataSizeP = (byte*)dataP + 4;
						memcpy(&dataSize, dataSizeP, sizeof(byte) * 4);
						break;
					}
				}

				if (!dataSize) return;
				else
				{
					UINT padddedDataSize = dataSize;
					padddedDataSize = ((dataSize / 16) + 1) * 16;
					sourceData = new byte[dataSize];
					byte* dataReadP = (byte*)dataP + 8;//Offset pointer past data name and size
					memcpy(sourceData, dataReadP, dataSize);
					sourceDataSize = dataSize;

					memcpy(&formatEX, formatP +8, sizeof(formatEX));

				}
				refCount = new int;
				*refCount = 1;

			}
			//Destructor
			~Wave()
			{
				*refCount -= 1;
				if (*refCount == 0)
					delete[] sourceData;

				if (*refCount < 1)
					delete refCount;
			}
			WAVEFORMATEX GetFormat() { return formatEX; }
			byte* GetDataP() { return sourceData; }
			UINT GetDataSize() { return sourceDataSize; }
			operator bool() { return *refCount; }
			void operator= (Wave otherWave)
			{
				this->~Wave();
				memcpy(this, &otherWave, sizeof(otherWave));
				//When object is live add reference
				//If not keep it null so memory can be freeded later
				if (*refCount > 0)
					*refCount += 1;
			}
		private:
			int* refCount = nullptr;
			byte* sourceData = nullptr;
			UINT sourceDataSize = 0;
			WAVEFORMATEX formatEX = { 0 };
		};
	}
}

namespace EGame
{
	namespace Audio
	{

		class Device
		{
		public:
			Device()
			{
				if (XAudio2Create(&xaudio2Device, XAUDIO2_DEBUG_ENGINE, XAUDIO2_DEFAULT_PROCESSOR) != S_OK)
					OutputDebugString(L"\n----Warning----\nXaudio failed to init!");
				else
				{
					XAUDIO2_DEBUG_CONFIGURATION debugConfig = { 0 };
					debugConfig.TraceMask = (XAUDIO2_LOG_ERRORS || XAUDIO2_LOG_WARNINGS);
					debugConfig.BreakMask = (XAUDIO2_LOG_ERRORS || XAUDIO2_LOG_WARNINGS);
					xaudio2Device->SetDebugConfiguration(&debugConfig);
					xaudio2Device->CreateMasteringVoice(&masterVoice);
					masterVoice->SetVolume(0.05);
				}
			}
			void SetMasterVolume(float volume)
			{
				masterVoice->SetVolume(volume);
			}
			HRESULT CreateSourceVoice(IXAudio2SourceVoice** outputVoice, WAVEFORMATEX waveFormat)
			{
				return xaudio2Device->CreateSourceVoice(outputVoice, &waveFormat);
			}
			HRESULT CreateSourceVoice(IXAudio2SourceVoice** outputVoice, WAVEFORMATEX waveFormat, IXAudio2VoiceCallback* callBack)
			{
				return xaudio2Device->CreateSourceVoice(outputVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, callBack, NULL, NULL);

			}
			~Device()
			{
				masterVoice->DestroyVoice();
			}
		private:
			CComPtr<IXAudio2> xaudio2Device;
			IXAudio2MasteringVoice* masterVoice = nullptr;
		}Device;

		//Call back handling class
		class SoundCallback : public IXAudio2VoiceCallback
		{
		public:
			SoundCallback() {}
			~SoundCallback() {}
			void __stdcall OnStreamEnd(){ }
			void __stdcall OnVoiceProcessingPassEnd() {  }
			void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) { }
			void __stdcall OnVoiceProcessingPassStart(UINT32 SamplesRequired) { }
			void __stdcall OnBufferEnd(void* pBufferContext) 
			{
			}
			void __stdcall OnBufferStart(void* pBufferContext)
			{
			}
			void __stdcall OnLoopEnd(void* pBufferContext) {    }
		};

		class Sound
		{
		public:
			Sound()
			{
				refCount = new int;
				*refCount = -1;
			}
			//Copy Constructor
			Sound(Sound& otherSound)
			{
				operator=(otherSound);
			}
			//----
			Sound(EGame::Audio::Wave& inputWave)
			{
				refCount = new int;

				if (!inputWave) return;
				HRESULT hr;
				WAVEFORMATEX format = inputWave.GetFormat();
				hr = Device.CreateSourceVoice(&soundVoice, format,&callBack);

				if (hr != S_OK)
					OutputDebugString(L"\nCouldn't make sound voice....");

				
				audioData = new byte[inputWave.GetDataSize()];
				memcpy(audioData, inputWave.GetDataP(), inputWave.GetDataSize());
				XAUDIO2_BUFFER xaudioBuffer = { 0 };
				xaudioBuffer.pAudioData = audioData;
				xaudioBuffer.AudioBytes = inputWave.GetDataSize();
				xaudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
				xaudioBuffer.Flags = XAUDIO2_END_OF_STREAM;

				hr = soundVoice->SubmitSourceBuffer(&xaudioBuffer);

				if (hr == S_OK)
				{
					*refCount = 1;
				}
			}
			~Sound()
			{
				*refCount -= 1;
				if (refCount < 0)
				{
					delete refCount;
					refCount = nullptr;
				}
			}
			void Start() 
			{
				if (*refCount == 0) return;
				soundVoice->Start();
			}
			void Stop()
			{
				soundVoice->Stop();
			}
			void SetVolume(float volume)
			{
				soundVoice->SetVolume(volume);
			}
			//A very important function to destroy a sound
			/*
			*Having a destructor was not good enough
			* as there's not enouhg time for the sound to be deleted.
			*/
			void DestroySound() 
			{
				*refCount -= 1;
				
				if (*refCount == 0)
				{
					soundVoice->DestroyVoice();
					delete[] audioData;
					*refCount = 0;
				}
			}
			void operator= (Sound& otherSound)
			{
				this->~Sound();
				*otherSound.refCount += 1;
				refCount = otherSound.refCount;
				audioData = otherSound.audioData;
				soundVoice = otherSound.soundVoice;
				memcpy(this, &otherSound, sizeof(Sound));
			}
		private:
			int* refCount = nullptr;
			byte* audioData;
			IXAudio2SourceVoice* soundVoice = nullptr;
			SoundCallback callBack;
		};
	}
}


