// languages.c  
// Last Modified: 4/20/2024
// Student names: Toby Nguyen
// language definitions for English and Spanish

typedef enum {English, Spanish} Language_t;
Language_t myL = English;
typedef enum {START, LANGUAGE, MULTIPLAYER, DEVMODE, CONTROLS, GAMEOVER, SCORE, MONEY, WAVE, P1, P2, GAMEMODE1, GAMEMODE2} phrase_t;
const char Start_English[] = "START";
const char Language_English[] = "Espa\xA4ol";
// const char Multiplayer_English[] = "Multiplayer";
const char Multiplayer_English[] = "Multiplayer";

const char Devmode__English[] = "Debug";
const char Controls_English[] = "to move \n to shoot and select \n to pause \n navigate menus";
const char GameOver_English[] = "Victory";
const char Score_English[] = "Score: ";
const char Money_English[] = "Money: ";
const char Wave_English[] = "Wave ";
const char P1_English[] = "P1";
const char P2_English[] = "P2";
const char Gamemode1_English[] = "Standard";
const char Gamemode2_English[] = "Flag Capture";


const char Start_Spanish[] = "INCIAR";
const char Language_Spanish[] = "English";
const char Multiplayer_Spanish[] = "Multijugador";
const char Devmode__Spanish[] = "Desarrollo";
const char Controls_Spanish[] = "moverse \n disparar o seleccionar \n parar \n navigar menus";
const char GameOver_Spanish[] = "Ganador";
const char Score_Spanish[] = "Puntuaci\xA2n: ";
const char Money_Spanish[] = "Dinero: ";
const char Wave_Spanish[] = "Nivel ";
const char P1_Spanish[] = "P1";
const char P2_Spanish[] = "P2";
const char Gamemode1_Spanish[] = "Standard";
const char Gamemode2_Spanish[] = "Capturar";

const char *Phrases[13][2] = {
     {Start_English, Start_Spanish},
     {Language_English, Language_Spanish},
     {Multiplayer_English, Multiplayer_Spanish},
     {Devmode__English, Devmode__Spanish},
     {Controls_English, Controls_Spanish},
     {GameOver_English, GameOver_Spanish},
     {Score_English, Score_Spanish},
     {Money_English, Money_Spanish},
     {Wave_English, Wave_Spanish},
     {P1_English, P1_Spanish},
     {P2_English, P2_Spanish},
     {Gamemode1_English, Gamemode1_Spanish},
     {Gamemode2_English, Gamemode2_Spanish},
};

