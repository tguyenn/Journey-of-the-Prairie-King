#ifndef LANGUAGES_H
#define LANGUAGES_H

extern const char Start_Spanish[];
extern const char Language_Spanish[];
extern const char Multiplayer_Spanish[];
extern const char Devmode__Spanish[];
extern const char Controls_Spanish[];
extern const char GameOver_Spanish[];
extern const char Score_Spanish[];
extern const char Money_Spanish[];
extern const char Wave_Spanish[];
extern const char P1_English[];
extern const char P2_English[];
extern const char Gamemode1_English[];
extern const char Gamemode2_English[];

extern const char Start_English[];
extern const char Language_English[];
extern const char Multiplayer_English[];
extern const char Devmode__English[];
extern const char Controls_English[];
extern const char GameOver_English[];
extern const char Score_English[];
extern const char Money_English[];
extern const char Wave_English[];
extern const char P1_Spanish[];
extern const char P2_Spanish[];
extern const char Gamemode1_Spanish[];
extern const char Gamemode2_Spanish[];

extern const char *Phrases[13][2];

typedef enum {English, Spanish} Language_t;
typedef enum {START, LANGUAGE, MULTIPLAYER, DEVMODE, CONTROLS, GAMEOVER, SCORE, MONEY, WAVE, P1, P2, GAMEMODE1, GAMEMODE2} phrase_t;

extern Language_t myL;

#endif
