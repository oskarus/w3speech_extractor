# w3speech_extractor
A tool for extracting audio from w3speech files

Works better than other extractors for expansions and non english dubs.
Extracted wav files need to be converted to ogg just like in https://github.com/Gizm000/Extracting-Voice-Over-Audio-from-Witcher-3

Instructions:
  1. download the repo
  2. g++ w3speech.cpp
  3. run the compiled script

Usage:
  ./a.out path_to_file language_code output_dir
  
  example: ./a.out ~/.wine/GOG\ Games/The\ Witcher\ 3\ Wild\ Hunt\ GOTY/dlc/bob/content/enpc.w3speech en audio
