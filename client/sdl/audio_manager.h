#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <string>

#include <SDL_mixer.h>

class AudioManager {
 private:
    Mix_Music* background_music;

 public:
    AudioManager();
    ~AudioManager();

    void play_background_music(const std::string& path, int volume);
    static void stop_music();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
};

#endif
