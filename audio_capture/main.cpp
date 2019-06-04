#include "fmod.hpp"
#include "fmod_errors.h"
#include "log.h"

#include <cstdint>
#include <thread>
#include <chrono>

#pragma comment(lib, "fmod_vc.lib")

int main()
{
	FMOD::System* system = NULL;
	FMOD::Sound* sound = NULL, * sound_to_play = NULL;
	FMOD::Channel* channel = 0;
	FMOD_RESULT result;
	uint32_t version;
	int numsubsounds;

	result = FMOD::System_Create(&system);
	if (result != FMOD_OK)
	{
		LOG("FMOD::System_Create() failed.\n");
		return 0;
	}

	result = system->getVersion(&version);
	if (result != FMOD_OK)
	{
		LOG("%s.\n", FMOD_ErrorString(result));
		return 0;
	}

	if (version < FMOD_VERSION)
	{
		LOG("FMOD lib version % 08x doesn't match header version %08x\N", version, FMOD_VERSION);
		return 0;
	}

	result = system->setOutput(FMOD_OUTPUTTYPE_DSOUND);
	if (result != FMOD_OK)
	{
		LOG("%s.\n", FMOD_ErrorString(result));
		return 0;
	}

	result = system->init(32, FMOD_INIT_NORMAL, NULL);
	if (result != FMOD_OK)
	{
		LOG("%s.\n", FMOD_ErrorString(result));
		return 0;
	}

	result = system->createStream("vlv.mp3", FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
	if (result != FMOD_OK)
	{
		LOG("%s.\n", FMOD_ErrorString(result));
		return 0;
	}

	result = sound->getNumSubSounds(&numsubsounds);
	if (result != FMOD_OK)
	{
		LOG("%s.\n", FMOD_ErrorString(result));
		return 0;
	}

	if (numsubsounds)
	{
		sound->getSubSound(0, &sound_to_play);
		if (result != FMOD_OK)
		{
			LOG("%s.\n", FMOD_ErrorString(result));
			return 0;
		}
	}
	else
	{
		sound_to_play = sound;
	}

	result = system->playSound(sound_to_play, 0, false, &channel);
	if (result != FMOD_OK)
	{
		LOG("%s.\n", FMOD_ErrorString(result));
		return 0;
	}

	bool isPlaying = true;
	while (1) 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		channel->isPlaying(&isPlaying);
		if (!isPlaying)
		{
			break;
		}
	}

	sound->release(); 
	system->close();
	system->release();

	getchar();
	return 0;
}