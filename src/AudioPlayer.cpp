#include "../include/AudioPlayer.hpp"
#include <SDL3/SDL_mixer.h>
#include <codecvt>
#include <exception>
#include <filesystem>
#include <string>

std::string convertToSystemEncoding(const std::wstring &wstr) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(wstr);
}
Sonic::SonicAudio::SonicAudio(std::filesystem::path path) {
  std::string pathStr = convertToSystemEncoding(path.wstring());
  Mix_Music *tmpMusic = nullptr;
  try {
    tmpMusic = Mix_LoadMUS(pathStr.c_str());
  } catch (std::exception &e) {
    std::cerr << "exception occured " << e.what() << " the path is " << path
              << std::endl;
  }
  if (!tmpMusic) {

    std::cerr << "Couldn't Load Music " << pathStr.c_str() << " \n";
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
    this->path = pathStr;
    duration = Mix_MusicDuration(tmpMusic);
    if (title.length() == 0)
      title = path.filename().string();
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

Sonic::SonicAudioPlayer::SonicAudioPlayer()
    : VOLUME(100), Current_Music(nullptr), freq(MIX_DEFAULT_FREQUENCY),
      channels(MIX_DEFAULT_CHANNELS), format(MIX_DEFAULT_FORMAT) {

  std::cout << "AudioPlayer starting with default settings\n";

  SDL_AudioSpec spec;
  spec.freq = freq;
  spec.channels = channels;
  spec.format = format;

  if (Mix_OpenAudio(0, &spec) < 0) {
    //
    std::cerr << "Couldn't Open Audio device\n";
    std::cerr << Mix_GetError() << std::endl;
    return;
  }
}
Sonic::SonicAudioPlayer::SonicAudioPlayer(int _frequency, int _channels,
                                          SDL_AudioFormat _format)
    : VOLUME(100), Current_Music(nullptr), freq(_frequency),
      channels(_channels), format(_format) {

  SDL_AudioSpec spec;
  spec.freq = freq;
  spec.channels = channels;
  spec.format = format;

  if (Mix_OpenAudio(0, &spec) < 0) {
    // do something
    //
    std::cerr << "Couldn't Open Audio device\n";
    std::cerr << Mix_GetError() << std::endl;
    exit(1);
  }

  // reseting the specifications based on the specs given by the audio device
  Mix_QuerySpec(&freq, &format, &channels);
  std::cerr << "Frequency: " << freq << "\n";
  std::cerr << "Format: " << (int)format << "\n";
  std::cerr << "Channels: " << channels << "\n";
}

Sonic::SonicAudioPlayer::~SonicAudioPlayer() {
  if (Current_Music)
    Mix_FreeMusic(Current_Music);
}
bool Sonic::SonicAudioPlayer::isSupported(std::string extention) {
  return (std::find(m_SupportedFormats.begin(), m_SupportedFormats.end(),
                    extention) != m_SupportedFormats.end());
}

void Sonic::SonicAudioPlayer::TogglePlay(const SonicAudio &audio) {
  if (Mix_PlayingMusic()) {
    // pause play back if it is the same song
    if (Current_SonicAudio.path == audio.path) {
      if (Mix_PausedMusic())
        Mix_ResumeMusic();
      else
        Mix_PauseMusic();
    } else {

      // TODO: maybe cross fading and other effects
      Mix_HaltMusic(); // brute force stopping
      Mix_FreeMusic(Current_Music);
      Current_Music = nullptr;
      Current_Music = Mix_LoadMUS(audio.path.c_str());

      Mix_PlayMusic(Current_Music, NUM_LOOPS);
    }
  } else {
    if (Current_Music)
      Mix_FreeMusic(Current_Music);
    Current_Music = nullptr;
    Current_Music = Mix_LoadMUS(audio.path.c_str());

    Mix_PlayMusic(Current_Music, NUM_LOOPS);
  }
  Current_SonicAudio = audio;
}

void Sonic::SonicAudioPlayer::setVolume(int vol) {
  vol = vol > (1 << 7) ? (1 << 7) : vol;
  vol = vol < 0 ? 0 : vol;
  VOLUME = vol;
  Mix_VolumeMusic(VOLUME);
}
int Sonic::SonicAudioPlayer::getVolume() { return VOLUME; }
double Sonic::SonicAudioPlayer::getCurrentAudioDuration() {
  return Mix_GetMusicPosition(Current_Music);
}
bool Sonic::SonicAudioPlayer::isAudioPlaying() { return Mix_PlayingMusic(); }

void Sonic::SonicAudioPlayer::seekCurrentAudio(double duration) {

  if (!Current_Music)
    return;
  duration = duration < 0 ? 0 : duration;

  if (duration >= Current_SonicAudio.duration) {
    /* Mix_HaltMusic(); */
    /* Mix_FreeMusic(Current_Music); */
    /* Current_Music = nullptr; */
  } else
    Mix_SetMusicPosition(duration);
}
