//#include "AccessProject_Beta09.ino"
int status_Select = 0;
boolean status_Insert = 0;
String registerType = "";

byte task_DBconnection()
{

  if (registertToFind != 0)
  {

    switch (step_DBQuery)
    {

      case DB_STEP_INITIAL:

        statusDB = 0;
        status_Select = DB_search(0, '\0', 0, 0);
        status_Insert = DB_insert(0, '\0', 0);
        step_DBQuery = DB_STEP_FIND_REGISTER;
        timer_DB_access = millis();

        break;

      case DB_STEP_FIND_REGISTER:

        status_Select = DB_search(1, DB_SEARCH_REGISTER, registertToFind, 0);

        if (status_Select == SUCCESS)
        {
          status_Select = DB_search(0, '\0', 0, 0);

          timer_DB_access = millis();

          if (myConfig.timeControl == false && myConfig.dateControl == false)
          {
            statusDB = DB_ALLOWED;
            step_DBQuery = DB_STEP_WAIT;
          }
          else
          {
            statusDB = DB_CHECKING_PARAMETERS;
            step_DBQuery = DB_STEP_CHECK_TYPE;
          }
        }
        else if (status_Select == FAILED)
        {

          errorDB = NOT_REGISTERED_ERROR;
          step_DBQuery = DB_STEP_ERROR;
        }
        else if ((millis() - timer_DB_access) >= MAX_TIME_SELECT)
        {
          errorDB = OVERFLOW_ERROR;
          step_DBQuery = DB_STEP_ERROR;
          lcd.clear();
          lcd.print("Iniciando");
          lcd.setCursor(0, 1);
          lcd.print("servidor...");
          boolean serverOK = 0;
          do
          {
            serverOK = function_DB_connect();
            delay(500);

          } while (!serverOK);
          lcd.clear();

          Serial.println("Iniciado server...");
        }
        break;

      case DB_STEP_CHECK_TYPE:

        registerType = DB_searchStr(1, DB_SEARCH_TYPE, registertToFind, 0);

        if (registerType.equalsIgnoreCase(FUNCIONARIO))
        {
          String s = DB_searchStr(0, '\0', 0, 0);
          timer_DB_access = millis();

          statusDB = DB_ALLOWED;
          step_DBQuery = DB_STEP_WAIT;
        }
        else if (registerType.equalsIgnoreCase(ALUNO) || registerType.equalsIgnoreCase(VISITANTE))
        {
          String s = DB_searchStr(0, '\0', 0, 0);
          timer_DB_access = millis();

          if (myConfig.dateControl)
          {
            statusDB = DB_CHECKING_PARAMETERS;
            step_DBQuery = DB_STEP_CHECK_DATE;
          }
          else
          {
            statusDB = DB_CHECKING_PARAMETERS;
            step_DBQuery = DB_STEP_CHECK_TIME;
          }
        }
        else if (registerType.equalsIgnoreCase("NOTHING"))
        {

          errorDB = DATE_ERROR;
          step_DBQuery = DB_STEP_ERROR;
        }
        else if ((millis() - timer_DB_access) >= MAX_TIME_SELECT)
        {
          errorDB = OVERFLOW_ERROR;
          step_DBQuery = DB_STEP_ERROR;
          lcd.clear();

          lcd.print("Iniciando");
          lcd.setCursor(0, 1);
          lcd.print("servidor...");
          boolean serverOK = 0;
          do
          {
            serverOK = function_DB_connect();
            delay(500);

          } while (!serverOK);
          lcd.clear();

          Serial.println("Iniciado server...");
        }

        break;

      case DB_STEP_CHECK_DATE:
        status_Select = DB_search(1, DB_SEARCH_DATE, registertToFind, 0);

        if (status_Select == SUCCESS)
        {
          status_Select = DB_search(0, '\0', 0, 0);

          if (myConfig.timeControl)
          {
            statusDB = DB_CHECKING_PARAMETERS;
            step_DBQuery = DB_STEP_CHECK_TIME;
          }
          else
          {
            statusDB = DB_ALLOWED;
            step_DBQuery = DB_STEP_WAIT;
          }

          timer_DB_access = millis();
        }
        else if (status_Select == FAILED)
        {

          errorDB = DATE_ERROR;
          step_DBQuery = DB_STEP_ERROR;
        }
        else if ((millis() - timer_DB_access) >= MAX_TIME_SELECT)
        {          
          errorDB = OVERFLOW_ERROR;
          step_DBQuery = DB_STEP_ERROR;
          lcd.clear();

          lcd.print("Iniciando");
          lcd.setCursor(0, 1);
          lcd.print("servidor...");
          boolean serverOK = 0;
          do
          {
            serverOK = function_DB_connect();
            delay(500);

          } while (!serverOK);
          lcd.clear();

          Serial.println("Iniciado server...");
        }

        break;

      case DB_STEP_CHECK_TIME:
        
        status_Select = DB_search(1, DB_SEARCH_TIME, registertToFind, 0);

        if (status_Select == SUCCESS)
        {
          status_Select = DB_search(0, '\0', 0, 0);

          statusDB = DB_ALLOWED;
          step_DBQuery = DB_STEP_WAIT;
        }
        else if (status_Select == FAILED)
        {
          errorDB = TIME_ERROR;
          step_DBQuery = DB_STEP_ERROR;
        }
        else if ((millis() - timer_DB_access) >= MAX_TIME_SELECT)
        {          
          errorDB = OVERFLOW_ERROR;
          step_DBQuery = DB_STEP_ERROR;
          lcd.clear();

          lcd.print("Iniciando");
          lcd.setCursor(0, 1);
          lcd.print("servidor...");

          boolean serverOK = 0;
          do
          {
            serverOK = function_DB_connect();
            delay(500);

          } while (!serverOK);
          lcd.clear();
          Serial.println("Iniciado server...");
        }

        break;

      case DB_STEP_WAIT:

        timer_DB_access = millis();

        break;

      case DB_STEP_REGISTER_ACCESS:

        if ((millis() - timer_DB_access) >= MAX_TIME_INSERT)
        {          
          statusDB = DB_ACCESS_NOT_REGISTERED;
          errorDB = OVERFLOW_ERROR;
          step_DBQuery = DB_STEP_ERROR;
        }

        if (status_Insert)
        {
          status_Insert = DB_insert(0, '\0', 0);
          registertToFind = 0;

          step_DBQuery = DB_STEP_INITIAL;
          statusDB = DB_ACCESS_REGISTERED;
        }

        break;

      case DB_STEP_ERROR:
        status_Select = DB_search(0, '\0', 0, 0);
        status_Insert = DB_insert(0, '\0', 0);
        statusDB = DB_NOT_ALLOWED;

        break;
    }
  }
  else
  {
    statusDB = 0;
    status_Select = DB_search(0, '\0', 0, 0);
    status_Insert = DB_insert(0, '\0', 0);
    timer_DB_access = millis();
  }
  return statusDB;
}

boolean function_DB_connect()
{
  Serial.println("Contatando o servidor...");

  if (db_connection.connect(server_addr, 3306, DB_USER, DB_PASSWORD))
  {
    delay(100);

    MySQL_Cursor *sql = new MySQL_Cursor(&db_connection);
    sql->execute(DB_NAME);
    delete sql;

    return 1;
  }
  db_connection.close();
  Serial.println("Servidor offline...tentando conexão novamente após 5 seg");
  return 0;
}

byte get_DBresponse()
{
  return statusDB;
}

void set_DB_target(long int value)
{
  if (value != 0)
  {
    registertToFind = value;
  }
  else
  {
    statusDB = 0;
    errorDB = 0;
    registertToFind = 0;
    step_DBQuery = DB_STEP_INITIAL;
  }
}

void set_DB_last_access(byte accessType)
{

  if (step_DBQuery == DB_STEP_WAIT)
  {
    step_DBQuery = DB_STEP_REGISTER_ACCESS;
  }

  if (step_DBQuery == DB_STEP_REGISTER_ACCESS)
  {
    if (accessType == ENTER)
    {

      status_Insert = DB_insert(1, DB_INSERT_LAST_ENTER, registertToFind);
    }
    else if (accessType == EXIT)
    {

      status_Insert = DB_insert(1, DB_INSERT_LAST_EXIT, registertToFind);
    }
  }
}
