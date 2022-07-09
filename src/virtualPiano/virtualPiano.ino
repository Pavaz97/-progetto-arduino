#include "pitches.h"
#include "WiFi.h"
#include "ThingSpeak.h"

#define LED_GREEN 4
#define LED_BLUE 5
#define LED_RED 2
#define BUZZER 23
#define NUMERO_MASSIMO_NOTE 100

const int TONE_PWM_CHANNEL = 1;
//const char* ssid = "FibraBrun7962";
//const char* password = "Mattia2020!";
const char* ssid = "TIM-25511286";
const char* password = "n4eMxTFPf7PAeLycx2k94Lh7";
//----------- Channel details ----------------//
unsigned long Channel_ID = 1666719; // Your Channel ID
const char * myWriteAPIKey = "Z277G945PR6L3488"; //Your write API key
//-------------------------------------------//
WiFiClient  client;
const int Field_Number_1 = 1;

int melodia [NUMERO_MASSIMO_NOTE] = {0};
int notaCorrente = 0;
bool suonaLive = false;

String message;

// Operazioni iniziali, eseguite una sola volta all'avvio di arduino
void setup() {
  // Inizializza connessione wifi e connessione a cloud, e definisce i pin di output
  ledcAttachPin(BUZZER, TONE_PWM_CHANNEL);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  Serial.begin(9600);
  InitConnection();
  ThingSpeak.begin(client);
}

// Operazione ripetuta all'infinito
void loop() {
  message = "";

  // Fino a quando non viene ricevuto alcun dato da bluetooth non viene eseguita alcuna elaborazione
  if (Serial.available() > 0) {
    // Se ho ricevuto un dato bluetooth, riempio la stringa message con ogni carattere ricevuto (finché c'è qualcosa da leggere)
    while (Serial.available()) {
      message += char(Serial.read());
    }

    // Elaboro il messaggio ricevuto
    elaboraDatoRicevutoConBluetooth();
  }
}

void elaboraDatoRicevutoConBluetooth () {
  char carattereDaValutare = toupper(message.charAt(0));

  // Possono arrivare caratteri di qualsiasi tipo. In base al carattere ricevuto decido come comporarmi.
  switch (carattereDaValutare) {
    // "L" indica che devo iniziare a suonare in live
    case 'L':
      suonaLive = true;
      break;
    // "." significa che devo finire di suonare in live (se stavo suonando in live).
    // Se non stavo suonando in live, devo riprodurre tutte le note ricevute fino ad ora.
    // In entrambi i casi eseguo il salvataggio su cloud di tutte le note suonate fino ad ora.
    case '.':
      if (suonaLive) {
        suonaLive = false;
      } else {
        riproduciCanzone();
      }
      salvaSuCloud();
      break;
    // "$" è un carattere speciale per suonare la canzone di StarWars.
    case '$':
      suonaStarWars();
      break;
    default:
      // Nel caso in cui il carattere ricevuto non sia nessuno dei precedenti, significa che potrebbe essere una nota.
      // Le note valide sono: A, B, C, D, E, F, G (Notazione internazionale per le note)
      // Ogni volta che arriva una nota valida la salvo in un array, e se sto suonando in live la riproduco immediatamente.
      if (isNotaValida(carattereDaValutare)) {
        melodia[notaCorrente] = convertiCarattereInFrequenza(carattereDaValutare);
        if (suonaLive) {
          riproduciNota(melodia[notaCorrente], 200);
        }
        notaCorrente++;
        break;
      }
  }
}

// Si occupa di riprodurre tutte le note salvate nell'array fino ad ora.
void riproduciCanzone() {
  for (int i = 0; i < NUMERO_MASSIMO_NOTE && melodia[i] > 0; i++) {
    riproduciNota(melodia[i], 500);
  }
}

boolean isNotaValida(char nota) {
  return nota == 'A' || nota == 'B' || nota == 'C' || nota == 'D' || nota == 'E' || nota == 'F' || nota == 'G';
}

int convertiCarattereInFrequenza(char carattereDaValutare) {
  switch (carattereDaValutare) {
    case 'A':
      return NOTE_A4;
      break;
    case 'B':
      return NOTE_B4;
      break;
    case 'C':
      return NOTE_C4;
      break;
    case 'D':
      return NOTE_D4;
      break;
    case 'E':
      return NOTE_E4;
      break;
    case 'F':
      return NOTE_F4;
      break;
    case 'G':
      return NOTE_G4;
      break;
    default:
      return 0;
      break;
  }
}

char convertiFrequenzaInCarattere(int frequenzaDaValutare) {
  switch (frequenzaDaValutare) {
    case NOTE_A4:
      return 'A';
      break;
    case NOTE_B4:
      return 'B';
      break;
    case NOTE_C4:
      return 'C';
      break;
    case NOTE_D4:
      return 'D';
      break;
    case NOTE_E4:
      return 'E';
      break;
    case NOTE_F4:
      return 'F';
      break;
    case NOTE_G4:
      return 'G';
      break;
    default:
      return 0;
      break;
  }
}

// Riprodurre una nota significa emettere una determinata frequenza al buzzer e far colorare il led RGB con un colore che sarà sempre uguale a parità di nota.
void riproduciNota(int nota, int durata)
{
  ledcWriteTone(TONE_PWM_CHANNEL, nota);
  coloraLed(nota);
  delay(durata);
  ledcWriteTone(TONE_PWM_CHANNEL, 0);
  delay(50);
}

// Vengono mandati i segnali analogici ai tre pin RGB per far colorare il led.
void coloraLed(int nota) {
  int colore = nota % 255;
  int coloreRosso = colore;
  int coloreVerde = colore;
  int coloreBlu = colore;
  if (colore >= 0 && colore <= 50) {
    coloreRosso = 0;
  }
  if (colore >= 51 && colore <= 150) {
    coloreVerde = 0;
  }
  if (colore > 150) {
    coloreBlu = 0;
  }

  analogWrite(LED_RED, coloreRosso);
  analogWrite(LED_GREEN, coloreVerde);
  analogWrite(LED_BLUE, coloreBlu);
}

// Viene riprodotta l'intera canzone di Star Wars
void suonaStarWars() {
  suonaPrimaSezioneStarWars();
  suonaSecondaSezioneStarWars();

  riproduciNota(NOTE_F4, 250);
  riproduciNota(NOTE_GS4, 500);
  riproduciNota(NOTE_F4, 350);
  riproduciNota(NOTE_A4, 125);
  riproduciNota(NOTE_C5, 500);
  riproduciNota(NOTE_A4, 375);
  riproduciNota(NOTE_C5, 125);
  riproduciNota(NOTE_E5, 650);

  analogWrite(LED_RED, 0);
  analogWrite(LED_GREEN, 0);
  analogWrite(LED_BLUE, 0);
  delay(650);
}

void suonaPrimaSezioneStarWars()
{
  riproduciNota(NOTE_A4, 500);
  riproduciNota(NOTE_A4, 500);
  riproduciNota(NOTE_A4, 500);
  riproduciNota(NOTE_F4, 350);
  riproduciNota(NOTE_C5, 150);
  riproduciNota(NOTE_A4, 500);
  riproduciNota(NOTE_F4, 350);
  riproduciNota(NOTE_C5, 150);
  riproduciNota(NOTE_A4, 650);

  delay(500);

  riproduciNota(NOTE_E5, 500);
  riproduciNota(NOTE_E5, 500);
  riproduciNota(NOTE_E5, 500);
  riproduciNota(NOTE_F5, 350);
  riproduciNota(NOTE_C5, 150);
  riproduciNota(NOTE_GS4, 500);
  riproduciNota(NOTE_F4, 350);
  riproduciNota(NOTE_C5, 150);
  riproduciNota(NOTE_A4, 650);

  delay(500);
}

void suonaSecondaSezioneStarWars()
{
  riproduciNota(NOTE_A5, 500);
  riproduciNota(NOTE_A4, 300);
  riproduciNota(NOTE_A4, 150);
  riproduciNota(NOTE_A5, 500);
  riproduciNota(NOTE_GS5, 325);
  riproduciNota(NOTE_G5, 175);
  riproduciNota(NOTE_FS5, 125);
  riproduciNota(NOTE_F5, 125);
  riproduciNota(NOTE_FS5, 250);

  delay(325);

  riproduciNota(NOTE_AS4, 250);
  riproduciNota(NOTE_DS5, 500);
  riproduciNota(NOTE_D5, 325);
  riproduciNota(NOTE_CS5, 175);
  riproduciNota(NOTE_C5, 125);
  riproduciNota(NOTE_B4, 125);
  riproduciNota(NOTE_C5, 250);

  delay(350);
}

void salvaSuCloud() {
  // Scrivo su cloud
  writeToCloud();

  // Ripulisco l'array melodia, l'indice dell'array e i colori del led RGB
  notaCorrente = 0;
  memset(melodia, 0, sizeof(melodia));
  analogWrite(LED_RED, 0);
  analogWrite(LED_GREEN, 0);
  analogWrite(LED_BLUE, 0);
}
void InitConnection() {
  int TentativiConnessione = 10;
  int ContatoreTentativi = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED && ContatoreTentativi <= TentativiConnessione) {
    Serial.print('.');
    delay(1000);
    ContatoreTentativi++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("connection successful");
  else
    Serial.println("Error connection");
}

void writeToCloud() {
  // write to the ThingSpeak channel
 char* prova = "Do RE";
 char* melodiaString;

 int x = ThingSpeak.writeField(Channel_ID, Field_Number_1, melodiaString, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    }
    else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
}
