//#include <Wire.h> //comunicação de dispositivo I2C
#include <SPI.h> //comunicação de dispositivo SPI
#include <SD.h> //biblioteca cartão SD
#include <DS1307.h> //biblioteca Real time clock
#include <DHT.h> //biblioteca Sensor DHT
#include "MQ7.h" //biblioteca sensor MQ7

#define DHTPIN 7     // pino DHT conectado na porta 7
#define DHTTYPE DHT11   // DHT 11  (AM2302)
MQ7 mq7(A1,5.0);

DHT dht(DHTPIN, DHTTYPE); // Inicializa o sensor DHT 16mhz
File arquivoTXT;
const int chipSelect = 10; //Porta CS do Módulo SD ligado à porta digital 10
DS1307 rtc(A2, A3); //Modulo RTC DS1307 ligado as portas A2 (SDA) e A3 (SCL) do Arduino

//Variáveis
//int chk;
float hum;  //Armazena valores de umidade
float temp; //Armazena valores de temperatura
int pin5 = 5; //Sensor DSM501a Vout2 MP1.0
int pin6 = 6; //Sensor DSM501a Vout1 MP2.5
int pinoR = 2; //Pino Digital em que o terminal 'R' está conectado
int pinoG = 3; //Pino Digital em que o terminal 'G' está conectado
int pinoB = 4; //Pino Digital em que o terminal 'B' está conectado

/***CONFIGURANDO SENSOR MP*/
unsigned long duration1;
unsigned long duration25;
unsigned long starttime;
unsigned long sampletime_ms = 3000;//sampe 3s ;
unsigned long lowpulseoccupancy1 = 0; //representa o Tempo de Ocupação de Pulso Baixo (Tempo LPO) detectado em determinados 30s. Sua unidade é microssegundos.
unsigned long lowpulseoccupancy25 = 0;

float ratio1 = 0; //reflete em qual nível o tempo de LPO ocupa todo o tempo de amostragem.
float ratio25 = 0;
float concentration1 = 0; //é uma figura que tem um significado físico. É calculado a partir do gráfico característico usando o tempo LPO.
float concentration25 = 0;


void setup() {

  Serial.begin(9600);

  /*****Definindo pinos MP*/
  pinMode(pin5, INPUT); //Pino digital 5 sensor MP
  pinMode(pin6, INPUT); //Pino digital 6 sensor MP
  starttime = millis();//get the current time;

/*-------------------------------------------------------------------*/
  
  //Wire.begin();
  SD.begin(10);      // o parametro e' o pino conectado ao CS do modulo
  dht.begin();
  pinMode(1, INPUT); // Valor digital sensor

/*-------------------------------------------------------------------*/

  /*****ACIONANDO O RELÓGIO*/
  rtc.halt(false);

  /****As linhas abaixo setam a data e hora do modulo
    e podem ser comentada apos a primeira utilizacao*/

  //rtc.setDOW(TUESDAY);      //Define o dia da semana
  //rtc.setTime(12, 54, 30);     //Define o horario
  //rtc.setDate(02, 8, 2022);   //Define o dia, mes e ano

  /*Definicoes do pino SQW/Out*/
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);
  
/*-------------------------------------------------------------------*/

//*****PROGRAMANDO SD CARD
  arquivoTXT = SD.open("data.txt", FILE_WRITE);
  if (arquivoTXT) {
    arquivoTXT.println("Data;Hora;Temperatura(ºC);Umidade(%);Concentracao CO(PPM);Concentracao CO(ug/m3);MP1.0(PPM);MP2.5(PPM)");
    arquivoTXT.close();
    Serial.println("Gravando dados.");
  } else {
    Serial.println("Erro ao abrir ou criar o arquivo texto.txt.");
    setColor(255, 255, 0); //cor do led amarela
  }
  Serial.println("card initialized.");
}

/*-------------------------------------------------------------------*/
/*FUNÇÃO QUE PRODUZ O BRILHO DE CADA UM DOS LEDS DE ACORDO COM OS PARÂMETROS INFORMADOS*/
void setColor(int vermelho, int verde, int azul) {

#ifdef COMMON_ANODE //SE O LED RGB FOR DEFINIDO COMO ANODO COMUM, FAZ
  vermelho = 255 - vermelho; //VARIÁVEL RECEBE O RESULTADO DA OPERAÇÃO '255 MENOS O PARÂMETRO (vermelho) INFORMADO' NA CHAMADA DA FUNÇÃO
  verde = 255 - verde; //VARIÁVEL RECEBE O RESULTADO DA OPERAÇÃO '255 MENOS O PARÂMETRO (verde) INFORMADO' NA CHAMADA DA FUNÇÃO
  azul = 255 - azul; //VARIÁVEL RECEBE O RESULTADO DA OPERAÇÃO '255 MENOS O PARÂMETRO (azul) INFORMADO' NA CHAMADA DA FUNÇÃO
#endif
  analogWrite(pinoR, vermelho); //DEFINE O BRILHO DO LED DE ACORDO COM O VALOR INFORMADO PELA VARIÁVEL 'vermelho'
  analogWrite(pinoG, verde); //DEFINE O BRILHO DO LED DE ACORDO COM O VALOR INFORMADO PELA VARIÁVEL 'verde'
  analogWrite(pinoB, azul); //DEFINE O BRILHO DO LED DE ACORDO COM O VALOR INFORMADO PELA VARIÁVEL 'azul'
}

//FUNÇÃO CONVERSÃO PPM PARA ug/m³
float converterPPM(float ppm) {
  float PesoMolec = 28.01;
  float Conc = ((ppm * PesoMolec * 1000) / 24.5);
  return Conc;
}

/*-------------------------------------------------------------------/


/*****MENU PRINCIPAL*/
void loop() {
  
  float mq7PPM = mq7.getPPM();                      //faz a leitura do sensor MQ7 em PPM
  float Concentracao = converterPPM(mq7PPM);        //faz a conversão de ppm para ug/m³
  float Resistencia = mq7.getSensorResistance();    //lê a resistência elétrica do sensor MQ7
  float Ratio = mq7.getRatio();                     //lê a taxa do sensor MQ7

  
  hum = dht.readHumidity();                         //Lê os valores de umidade e temperatura e armazena em hum e temp
  temp = dht.readTemperature();
  
  if (isnan(hum) || isnan(temp)) {                  // Verifica se o sensor DHT22 esta respondendo
    Serial.println("Falha ao ler dados do sensor DHT !!!");
    setColor(255, 0, 0); //COR VERMELHA
    delay(2000); //INTERVALO DE 2 SEGUNDOS
  }

  //Leitura da data e hora
  Serial.print("Data: ");
  Serial.print(rtc.getDateStr(FORMAT_SHORT));
  Serial.print(" ");
  Serial.print("Hora: ");
  Serial.print(rtc.getTimeStr());
  Serial.print(" ");
  Serial.println(rtc.getDOWStr(FORMAT_SHORT));

  Serial.print(F("Concentração CO (ppm): "));
  Serial.println(mq7PPM,2);

  Serial.print(F("Concentração CO (ug/m3): "));
  Serial.println(Concentracao);

  //Serial.print("Res: ");
  //Serial.println(Resistencia);

  //Serial.print("Ratio: ");
  //Serial.println(Ratio);

  Serial.print(F("Temperatura (C): "));
  Serial.println(temp, 2); // Mostra a temperatura com 2 casas decimais

  Serial.print(F("Umidade (%): "));
  Serial.println(hum);

  /*Leitura Sensor MP*/
  duration1 = pulseIn(pin5, LOW);
  duration25 = pulseIn(pin6, LOW);
  lowpulseoccupancy1 = lowpulseoccupancy1 + duration1;
  lowpulseoccupancy25 = lowpulseoccupancy25 + duration25;

  if ((millis() - starttime) > sampletime_ms) //if the sampel time == 30s
  {
    ratio1 = lowpulseoccupancy1 / (sampletime_ms * 10.0); // Integer percentage 0=>100
    concentration1 = 1.1 * pow(ratio1, 3) - 3.8 * pow(ratio1, 2) + 520 * ratio1 + 0.62; // using spec sheet curve

    ratio25 = lowpulseoccupancy25 / (sampletime_ms * 10.0); // Integer percentage 0=>100
    concentration25 = 1.1 * pow(ratio25, 3) - 3.8 * pow(ratio25, 2) + 520 * ratio25 + 0.62; // using spec sheet curve

    //  Serial.print(F("Tempo de Ocupação Pulso Baixo (MP1.0): "));
    //  Serial.print(lowpulseoccupancy1);
    //  Serial.println(F("ms"));
    //  Serial.print(F("Taxa (MP1.0): "));
    //  Serial.println(ratio1);
    //  Serial.print(F(", %t"));
    Serial.print(F("Concentração MP1.0: "));
    Serial.println(concentration1);
    lowpulseoccupancy1 = 0;

    //  Serial.print(F("Tempo de Ocupação Pulso Baixo (MP2.5): "));
    //  Serial.print(lowpulseoccupancy25);
    //  Serial.println(F("ms"));
    //  Serial.print(F("Taxa (MP2.5): "));
    //  Serial.println(ratio25);
    //  Serial.print(F(", %t"));
    Serial.print(F("Concentração MP2.5: "));
    Serial.println(concentration25);
    lowpulseoccupancy25 = 0;

    starttime = millis();
  } 

  Serial.println(F("------------------------"));

  File arquivoTXT = SD.open("data.txt", FILE_WRITE);

  if (arquivoTXT) {

    arquivoTXT.print(rtc.getDateStr(FORMAT_SHORT));
    arquivoTXT.print(";");
    arquivoTXT.print(rtc.getTimeStr());
    arquivoTXT.print(";");
    arquivoTXT.print(temp, 2); // Mostra a temperatura com 2 casas decimais
    arquivoTXT.print(";");
    arquivoTXT.print(hum);
    arquivoTXT.print(";");
    arquivoTXT.print(mq7PPM);
    arquivoTXT.print(";");
    arquivoTXT.print(Concentracao);
    arquivoTXT.print(";");
    arquivoTXT.print(concentration1);
    arquivoTXT.print(";");
    arquivoTXT.println(concentration25);
  }
  else {
    Serial.println("error opening data.txt");
    setColor(255, 100, 0); //COR amarela
    delay(2000); //INTERVALO DE 2 SEGUNDOS
  }
  arquivoTXT.close();

  if (!isnan(mq7PPM) and !isnan(concentration1) and !isnan(concentration25)) {
    setColor(0, 255, 0); //Cor verde
    delay(1000); //INTERVALO DE 1 SEGUNDO
  }

  setColor(0, 0, 0); //led apagado
  delay(30000); //Intervalo de 30 segundos
}
