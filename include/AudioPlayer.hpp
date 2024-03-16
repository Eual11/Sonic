#ifndef _AUDIOPLAYER_HPP
#define _AUDIOPLAYER_HPP
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_mixer.h>
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
  SonicAudio(std::string path);
  SonicAudio(const SonicAudio &audio);            // copy constructor
  SonicAudio &operator=(const SonicAudio &audio); // assignment operator
  SonicAudio(SonicAudio &&audio) noexcept;        // move constructor

private:
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
  SonicAudioPlayer(SonicAudio);
  SonicAudioPlayer(int, int, SDL_AudioFormat);
  SonicAudioPlayer(Mix_Music *);
  ~SonicAudioPlayer(void);
  // getter and setter methods for player audio
  int getVolume(void);
  void setVolume(int vol);

  // duration and seeking operations
  double getCurrentAudioDuration();
  void seekCurrentAudio(double);
  void forwardCurrentAudio(double);
  void backwardCurrentAudio(double);

  // repeating
  void toggleRepeat();
  void Play();
  void rewindCurrentAudio();
  void pauseCurrentAudio();
  void ResumeCurrentAudio();

private:
  int VOLUME;
  Mix_Music *Current_Audio;
  int freq;
  int sample_size;
  int channels;
  SDL_AudioFormat format;
};
}; // namespace Sonic
#endif
