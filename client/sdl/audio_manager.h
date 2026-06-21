#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <map>
#include <string>

#include <SDL_mixer.h>

class AudioManager {
 private:
    Mix_Music* background_music;
    std::map<std::string, Mix_Chunk*> sounds;

 public:
    AudioManager();
    ~AudioManager();

    void play_background_music(const std::string& path, int volume);
    static void stop_music();
    static void toggle_music();
    static bool is_music_paused();

    void load_sound(const std::string& name, const std::string& path);
    void play_sound(const std::string& name, int volume = MIX_MAX_VOLUME);

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
};

#endif
