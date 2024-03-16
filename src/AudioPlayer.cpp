#include "../include/AudioPlayer.hpp"
#include <SDL3/SDL_mixer.h>

Sonic::SonicAudio::SonicAudio(std::string path) {
  Mix_Music *tmpMusic = Mix_LoadMUS(path.c_str());
  if (!tmpMusic) {

    std::cerr << "Couldn't Load Music\n";
    std::cerr << Mix_GetError() << "\n";
    title = "";
    artist = "";
    album = "";
    duration = 0;
    this->path = "";
  } else {

    title = Mix_GetMusicTitle(tmpMusic);
    artist = Mix_GetMusicArtistTag(tmpMusic);
    album = Mix_GetMusicAlbumTag(tmpMusic);
    this->path = path;
    duration = Mix_MusicDuration(tmpMusic);
    if (artist.length() == 0)
      artist = "Unkown Artist";
    if (album.length() == 0)
      album = "Unkown Album";
    Mix_FreeMusic(tmpMusic);
  }
}
Sonic::SonicAudio::SonicAudio() {
  title = "";
  artist = "";
  album = "";
  duration = 0;
  path = "";
}
Sonic::SonicAudio::SonicAudio(const Sonic::SonicAudio &audio) {
  this->album = audio.album;
  this->artist = audio.artist;
  this->path = audio.path;
  this->duration = audio.duration;
  this->title = audio.title;
}

Sonic::SonicAudio &Sonic::SonicAudio::operator=(const SonicAudio &audio) {

  if (this == &audio) {
    return *this;
  }
  this->album = audio.album;
  this->artist = audio.artist;
  this->path = audio.path;
  this->duration = audio.duration;
  this->title = audio.title;

  return *this;
}
Sonic::SonicAudio::SonicAudio(Sonic::SonicAudio &&audio) noexcept {
  this->title = std::move(audio.title);
  this->path = std::move(audio.path);
  this->artist = std::move(audio.artist);
  this->album = std::move(audio.album);
  this->duration = audio.duration;
}
