
/***********************************************/
/******* PROJECT: SENAI ACCESS CONTROL *********/
/*---------------------------------------------*/
/*Autor: André Felipe Savedra Cruz(CFP 501)    */
/*Data: 19.02.20                               */
/*Revisão: 06.11.20(Nova Eletrônica e Leitor)  */
/*Revisão: 05.01.20(Configs do Firmware)       */
/*---------------------------------------------*/
/***********************************************/

/******LIBRARIES:*********/
#include <SPI.h>
#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <LiquidCrystal.h>

/***ETHERTNET PARAMETERS****/
byte ip[] = {192, 168, 10, 8};
byte myDns[] = {192, 168, 10, 254};

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};


IPAddress server_addr(192, 168, 10, 10);
EthernetClient client;
String schoolName_1 = "Senai Campinas"; //LINHA1: MÁXIMO 16 CARACTERES E SEM ACENTUACAO
String schoolName_2 = "Passe o Cracha..";  //LINHA2: MÁXIMO 16 CARACTERESES E SEM ACENTUACAO
// String schoolName_2 = "Roberto Mange";  //LINHA2: MÁXIMO 16 CARACTERESES E SEM ACENTUACAO

/***ACCESS MANAGER VARIABLES:****/

#define NORMALLY_BLOCKED 0 //Solenóide fica normalmente bloqueada, somente acionada para liberar o acesso
#define NORMALLY_RELEASED 1 //Solenóide fica normalmente liberada, somente acionada para bloquear o acesso
boolean solenoidLogic = NORMALLY_RELEASED; //MUDE AQUI PARA ALTERAR LÓGICA DA SOLENÓIDE DA CATRACA!!!!
boolean solenoidBlockedByMovement = false;
boolean oldSensor1 = false, oldSensor2 = false;

#define SENSOR_HOME_POSITION 1 // 0 se sensor em home position está desativado e 1 se está ativado
                               // catraca antiga: 0, catraca nova: 1
#define scan_task_accessManager 10
#define error_time 3000

#define solenoidPin 53 //PIN
#define errorLedPin 49 //PIN
#define ok_LedPin 51   //PIN

#define STEP_INITIAL 0
#define STEP_STANDBY 1
#define STEP_CHECK_DB_NUMBER 2
#define STEP_CHECK_DB_DATETIME 3
#define STEP_ALLOWED 4
#define STEP_REGISTER_DIRECTION 5
#define STEP_NOT_ALLOWED 6

#define JUST_CHECK_REGISTER_MODE 1

boolean solenoid = 0, ok_Led = 0, errorLed = 0, mode = 0;

byte step_AccessManager = 0, DB_response = 0;
byte directionAccess = 0;

unsigned long timer_task_accessManager = 0, timer_error_accessManager = 0;

long int codeRead = 0, findThis = 0;
long int registerNumber = 0;

/***CODE READER VARIABLES:****/
#define wait_receiveData 100

unsigned long numberRead = 0, timer_Data = 0;

char buffer[40];
int numberChar = 0;
boolean bufferRead = 0;

/***DATABASE CONNECTION VARIABLES:****/
MySQL_Connection db_connection((Client *)&client);

#define DB_NONE 0
#define DB_SEARCHING 1
#define DB_CHECKING_PARAMETERS 2
#define DB_ALLOWED 3
#define DB_NOT_ALLOWED 4
#define DB_ACCESS_REGISTERED 5
#define DB_ACCESS_NOT_REGISTERED 6

#define DB_ERROR 6

#define DB_STEP_INITIAL 0
#define DB_STEP_FIND_REGISTER 1
#define DB_STEP_CHECK_TYPE 2
#define DB_STEP_CHECK_DATE 3
#define DB_STEP_CHECK_TIME 4
#define DB_STEP_WAIT 5
#define DB_STEP_REGISTER_ACCESS 6
#define DB_STEP_ERROR 7

#define DB_CONFIG_INITIAL 0
#define DB_CONFIG_READ_RELEASE 1
#define DB_CONFIG_READ_CONTROLTIME 2
#define DB_CONFIG_READ_CONTROLDATE 3
#define DB_CONFIG_RESET 4
#define DB_CONFIG_END 5

#define MAX_TIME_SELECT 7000 //7segundos
#define MAX_TIME_INSERT 7000 //7segundos

#define MAX_TIME_READ_CONFIG 7000 //7segundos

unsigned long TIME_REFRESH = 1200000; //20minutos

#define NOTHING 20
#define SUCCESS 1
#define FAILED -1

#define ALUNO "aluno"
#define VISITANTE "visitante"
#define FUNCIONARIO "funcionario"

unsigned long timer_DB_access = 0, timerReadConfigs = 0, timerRefreshConfigs = 0;
boolean serverOK = 0;
byte step_DBQuery = 0, statusDB = 0, stepInsert = 0, step_DB_Configs = 0;
long int registertToFind = 0;
boolean bufferRelease = false;

unsigned long timerDelayAllowed = 0;
bool flagDelayAllowed = false;

struct CONFIGS
{
  bool release;
  bool timeControl;
  bool dateControl;
};

CONFIGS myConfig = {false, false, false};

row_values *row = NULL;

char DB_USER[] = "savedra";
char DB_PASSWORD[] = "savedra";

char DB_NAME[] = "USE escola";
char DB_SEARCH_REGISTER[] = "SELECT IF ( ((SELECT matricula FROM registros WHERE matricula = (%ld)) > 0), 1, -1)";
char DB_SEARCH_TYPE[] = "SELECT tipoCadastro FROM registros WHERE matricula = (%ld)";

char DB_SEARCH_DATE[] = "SELECT checaData(%ld)";
char DB_SEARCH_TIME[] = "SELECT checaHorario(%ld)";

char DB_SEARCH_FINAL_TIME[] = "SELECT round((time_to_sec((SELECT TIMEDIFF(CURTIME(),(select horaFinal from alunos where matriculaFK = (%ld)) )) ) / 60))";
char DB_SEARCH_BIRTH[] = "SELECT TIMESTAMPDIFF(YEAR, (SELECT dataNascimento FROM alunos WHERE matriculaFK = (%ld)), curdate() )";

char DB_INSERT_LAST_ENTER[] = "call atualizacao((%ld),1)";
char DB_INSERT_LAST_EXIT[] = "call atualizacao((%ld),2)";

char DB_SEARCH_RELEASE[] = "SELECT liberar from config where id = (%ld)";
char DB_SEARCH_TIME_CONTROL[] = "SELECT controle_hora from config where id = (%ld)";
char DB_SEARCH_DATE_CONTROL[] = "SELECT controle_data from config where id = (%ld)";
char DB_SEARCH_TOL_TIMEIN[] = "SELECT tol_hora_entrada from config where id = (%ld)";
char DB_SEARCH_TOL_TIMEOUT[] = "SELECT tol_hora_saida from config where id = (%ld)";
char DB_SEARCH_TOL_DATEIN[] = "SELECT tol_data_inicio from config where id = (%ld)";
char DB_SEARCH_TOL_DATEOUT[] = "SELECT tol_data_fim from config where id = (%ld)";
//char DB_SEARCH_SCHOOL_NAME[] = "SELECT escola from config where id = (%ld)";

byte errorDB = 0;

/***IHM:****/
LiquidCrystal lcd(9, 8, 13, 12, 11, 10); //PINS

#define NOT_REGISTERED_ERROR 1
#define TIME_ERROR 2
#define DATE_ERROR 3
#define OVERFLOW_ERROR 4
#define ACCESS_REGISTER_ERROR 5
#define OVERFLOW_ACCESS_ERROR 6

/***SENSORS VARIABLES:****/
#define sensor1Pin 22 //PIN
#define sensor2Pin 23 //PIN

#define maxTime_Movement 15000 //15segundos

#define ENTER 1
#define EXIT 2

#define MOVEMENT_STARTED 1
#define MOVEMENT_COMPLETED 2
#define MOVEMENT_OVERFLOW 3

boolean buffer_sensors = 0, sensor1 = 0, sensor2 = 0;
byte step_sensors = 0;

unsigned long timer_movement = 0;

void (*funcReset)() = 0;

void setup()
{
  delay(1000);
  mode = JUST_CHECK_REGISTER_MODE;
  /***SERIAL INITIALIZATION:****/
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Iniciando...");

  /***IHM INITIALIZATION:****/
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Iniciando");
  lcd.setCursor(0, 1);
  lcd.print("servidor...");

  /***DATABASE INITIALIZATION:****/

  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  else {
    Ethernet.begin(mac, ip, myDns);
    delay(2000);
    // print your local IP address:
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());

  }

  boolean serverOK = 0;
  Serial.print(Ethernet.localIP());
  serverOK = function_DB_connect();
  delay(500);
  if (!serverOK)
  {
    funcReset();
  }

  Serial.println("Iniciado Servidor...");

  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("Servidor");
  lcd.setCursor(0, 1);
  lcd.print("conectado...");
  delay(2000);
  lcd.clear();

  /***CODE READER INITIALIZATION:****/
  Serial1.begin(115200);

  /***I/O's INITIALIZATION:****/
  pinMode(solenoidPin, OUTPUT);
  pinMode(errorLedPin, OUTPUT);
  pinMode(ok_LedPin, OUTPUT);
  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);

  solenoid = 0;
  ok_Led = 0;

  digitalWrite(solenoidPin, LOW);
  digitalWrite(errorLedPin, LOW);
  digitalWrite(ok_LedPin, LOW);

  /***VARIABLES INITIALIZATION:****/
  step_AccessManager = STEP_INITIAL;
  numberRead = 0;

  /***READING CONFIGURATIONS ONCE***/
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lendo");
  lcd.setCursor(0, 1);
  lcd.print("configuracoes..");

  timerReadConfigs = millis();
  bool buffConfig = false;

  do
  {
    if ((millis() - timerReadConfigs) > MAX_TIME_READ_CONFIG)
    {
      step_DB_Configs = DB_CONFIG_END;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Erro Leitura");
      lcd.setCursor(0, 1);
      lcd.print("configuracoes");
      delay(2000);
      buffConfig = true;
    }
    else
    {
      task_ReadConfigs();
    }

  } while (step_DB_Configs != DB_CONFIG_END);

  if (buffConfig == false)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Configuracoes");
    lcd.setCursor(0, 1);
    lcd.print("OK!");
    delay(2000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando");
  lcd.setCursor(0, 1);
  lcd.print("catraca...");
  delay(2000);
  lcd.clear();

  timerRefreshConfigs = millis();
  //Serial.println(myConfig.release);
  //Serial.println(myConfig.timeControl);
  //Serial.println(myConfig.dateControl);

   oldSensor1 = sensor1;
   oldSensor2 = sensor2;
} //end setup

void loop()
{

  if (myConfig.release == false && bufferRelease == false)
  {
    if ((millis() - timer_task_accessManager) >= scan_task_accessManager)
    {
      task_accessManager();
      timer_task_accessManager = millis();
    }
    TIME_REFRESH = 1200000; //20minutos
  }

  if (myConfig.release == true)
  {

    if (bufferRelease == false)
    {
      lcd.clear();
    }

    bufferRelease = true;
  }

  if (bufferRelease == true)
  { //release access
    solenoid = 1;
    TIME_REFRESH = 60000; //1minuto

    if (myConfig.release == false)
    {
      bufferRelease = false;

      step_AccessManager = STEP_INITIAL;
      step_DBQuery = DB_STEP_INITIAL;
    }
  }


  //FOR DIFFERENT SOLENOIDS TYPE
  if (solenoidLogic == NORMALLY_RELEASED)
  {
    digitalWrite(solenoidPin, solenoid);
  }
  else
  {
    digitalWrite(solenoidPin, !solenoid);
  }

  digitalWrite(errorLedPin, errorLed);
  digitalWrite(ok_LedPin, ok_Led);

  sensor1 = !digitalRead(sensor1Pin);
  sensor2 = !digitalRead(sensor2Pin);

  Serial.print("Sensor1: ");
  Serial.println(sensor1);

  Serial.print("Sensor2: ");
  Serial.println(sensor2);

  //refresh configurations
  if ((millis() - timerRefreshConfigs) > TIME_REFRESH)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lendo");
    lcd.setCursor(0, 1);
    lcd.print("configuracoes..");

    timerReadConfigs = millis();
    bool buffConfig = false;

    do
    {
      if ((millis() - timerReadConfigs) > MAX_TIME_READ_CONFIG)
      {
        step_DB_Configs = DB_CONFIG_END;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Erro Leitura");
        lcd.setCursor(0, 1);
        lcd.print("configuracoes");
        delay(2000);
        buffConfig = true;
      }
      else
      {
        task_ReadConfigs();
      }

    } while (step_DB_Configs != DB_CONFIG_END);

    step_DB_Configs = DB_CONFIG_INITIAL;

    if (buffConfig == false)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Configuracoes");
      lcd.setCursor(0, 1);
      lcd.print("OK!");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(schoolName_1);
      lcd.setCursor(0, 1);
      lcd.print(schoolName_2);
    }

    timerRefreshConfigs = millis();

  } //end refresh configurations
  else
  {
    if (myConfig.release == true)
    {

      lcd.setCursor(0, 0);
      lcd.print("Catraca        ");
      lcd.setCursor(0, 1);
      lcd.print("Liberada!      ");
    }
  }
}
