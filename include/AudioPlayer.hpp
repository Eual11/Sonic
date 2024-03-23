#ifndef _AUDIOPLAYER_HPP
#define _AUDIOPLAYER_HPP
#include "../include/Utils.hpp"
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_mixer.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
/*
 * audio class should have
 * title: a name crossponding to the audio title
 * artist: whoeever created it
 * album: orgin album
 * duration: total duration
 * Playlist: name of playlist it is from //could be unused
 * path: the music path to be played from
 *
 * System.sav*/

namespace Sonic {

class SonicAudio {
public:
  SonicAudio(); // default constructor
  SonicAudio(std::filesystem::path path);
  SonicAudio(const SonicAudio &audio);            // copy constructor
  SonicAudio &operator=(const SonicAudio &audio); // assignment operator
  SonicAudio(SonicAudio &&audio) noexcept;        // move constructor

  std::string title;  // audio title
  std::string artist; // artist
  std::string album;
  double duration;
  std::vector<std::string> playlist;
  std::string path;
};

class SonicAudioPlayer {

public:
  SonicAudioPlayer();
  SonicAudioPlayer(int, int, SDL_AudioFormat);
  ~SonicAudioPlayer(void);
  // getter and setter methods for player audio
  int getVolume(void);
  void setVolume(int vol);

  bool isSupported(std::string);
  // duration and seeking operations
  double getCurrentAudioDuration();
  void seekCurrentAudio(double);
  inline void forwardCurrentAudio(double amount = 5.0f) {
    Sonic::SonicAudioPlayer::seekCurrentAudio(
        Mix_GetMusicPosition(Current_Music) + amount);
  };
  inline void backwardCurrentAudio(double amount = 5.0f) {
    Sonic::SonicAudioPlayer::seekCurrentAudio(
        Mix_GetMusicPosition(Current_Music) - amount);
  }

  // repeating
  inline void toggleRepeat();
  void TogglePlay(const Sonic::SonicAudio &);
  inline void rewindCurrentAudio() {
    backwardCurrentAudio(Current_SonicAudio.duration);
  }
  inline void toggleLoop() { NUM_LOOPS = NUM_LOOPS < 0 ? 0 : -1; }

  void ResumeCurrentAudio();
  bool isAudioPlaying();
  inline bool isPaused() { return Mix_PausedMusic(); }
  int VOLUME;
  bool m_Shuffle = false;
  bool m_Loop = false;

private:
  Mix_Music *Current_Music;
  SonicAudio Current_SonicAudio;
  int NUM_LOOPS = 0;
  int freq;
  int sample_size;
  int channels;
  SDL_AudioFormat format;
  std::vector<std::string> m_SupportedFormats = {".mp3", ".wav", ".ogg",
                                                 ".flac"};
};
}; // namespace Sonic
#endif
