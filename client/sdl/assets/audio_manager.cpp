#include "audio_manager.h"

#include <algorithm>
#include <stdexcept>

AudioManager::AudioManager(): background_music(nullptr) {
    const int requested_formats = MIX_INIT_OGG;

    if ((Mix_Init(requested_formats) & requested_formats) != requested_formats) {
        throw std::runtime_error(std::string("No se pudo inicializar SDL_mixer: ") +
                                 Mix_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        Mix_Quit();

        throw std::runtime_error(std::string("No se pudo abrir el audio: ") + Mix_GetError());
    }
}

AudioManager::~AudioManager() {
    Mix_HaltMusic();
    Mix_HaltChannel(-1);

    if (background_music != nullptr) {
        Mix_FreeMusic(background_music);
        background_music = nullptr;
    }

    for (auto& [name, chunk] : sounds) {
        Mix_FreeChunk(chunk);
    }
    sounds.clear();

    Mix_CloseAudio();
    Mix_Quit();
}

void AudioManager::play_background_music(const std::string& path, int volume) {
    if (background_music != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(background_music);
        background_music = nullptr;
    }

    background_music = Mix_LoadMUS(path.c_str());

    if (background_music == nullptr) {
        throw std::runtime_error(std::string("No se pudo cargar la música: ") + Mix_GetError());
    }

    const int safe_volume = std::clamp(volume, 0, MIX_MAX_VOLUME);

    Mix_VolumeMusic(safe_volume);

    // el -1 hace que la musica se repita continuamente
    if (Mix_PlayMusic(background_music, -1) < 0) {
        throw std::runtime_error(std::string("No se pudo reproducir la música: ") + Mix_GetError());
    }
}

void AudioManager::stop_music() {
    Mix_HaltMusic();
}

void AudioManager::toggle_music() {
    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    } else {
        Mix_PauseMusic();
    }
}

bool AudioManager::is_music_paused() {
    return Mix_PausedMusic() != 0;
}

void AudioManager::load_sound(const std::string& name, const std::string& path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (chunk == nullptr) {
        return;
    }
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        Mix_FreeChunk(it->second);
    }
    sounds[name] = chunk;
}

void AudioManager::play_sound(const std::string& name, int volume) {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        return;
    }
    const int safe_volume = std::clamp(volume, 0, MIX_MAX_VOLUME);
    Mix_VolumeChunk(it->second, safe_volume);
    Mix_PlayChannel(-1, it->second, 0);
}
