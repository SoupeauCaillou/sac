/*!
 * \file SoundSystem.h
 * \brief All that you need to add sound in a game
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <list>
#include <algorithm>

#include "System.h"
#include "api/SoundAPI.h"

typedef int SoundRef;

#define InvalidSoundRef -1

/*! \struct SoundComponent
 *  \brief ? */
struct SoundComponent {
	SoundComponent() : sound(InvalidSoundRef), volume(1) { }
	SoundRef sound; //!< ?
	float volume; //!< volume of sound
};

#define theSoundSystem SoundSystem::GetInstance()
#define SOUND(e) theSoundSystem.Get(e)

/*! \class Sound
 *  \brief All that you need to add sound in a game */
UPDATABLE_SYSTEM(Sound)

public:
~SoundSystem();
/*! \brief initialization of something */
void init();

/*! \brief Load a sound file
 *  \param assetName : name of the sound file
 *  \return Returns a SoundRef */
SoundRef loadSoundFile(const std::string& assetName);

private:
/* textures cache */
SoundRef nextValidRef; //!< ?
std::map<std::string, SoundRef> assetSounds; //!< match between sound name and sound reference
std::map<SoundRef, OpaqueSoundPtr*> sounds; //!< match between sound reference and ?

public:
SoundAPI* soundAPI; //!< pointer to sound API (for PC or Android or ...)
bool mute; //!< State of the sound (mute = true)

};

