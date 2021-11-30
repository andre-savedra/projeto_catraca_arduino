//#include "CATRACA3-Nova.ino"

void task_accessManager()
{
  codeRead = code_reader();
  //codeRead = 1051443;

  if ((codeRead != 0) && (!findThis))
  {
    findThis = codeRead;
    Serial.println(codeRead);
    lcd.clear();
  }

  switch (step_AccessManager)
  {

  case STEP_INITIAL:
    solenoid = 0;
    ok_Led = 0;
    errorLed = 0;

    findThis = 0;
    DB_response = 0;
    reset_Sensors();
    errorDB = 0;

    step_AccessManager = STEP_STANDBY;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(schoolName_1);
    lcd.setCursor(0, 1);
    lcd.print(schoolName_2);
    break;

  case STEP_STANDBY:

    if (findThis > 0)
    {
      step_AccessManager = STEP_CHECK_DB_NUMBER;

      set_DB_target(findThis);
    }

    break;

  case STEP_CHECK_DB_NUMBER:
    DB_response = get_DBresponse();

    if (DB_response == DB_CHECKING_PARAMETERS)
    {
      step_AccessManager = STEP_CHECK_DB_DATETIME;
    }
    else if (DB_response == DB_NOT_ALLOWED)
    {

      step_AccessManager = STEP_NOT_ALLOWED;
      timer_error_accessManager = millis();
      lcd.clear();
      if (errorDB == OVERFLOW_ERROR)
      {
        lcd.setCursor(0, 0);
        lcd.print("Servidor Offline");
        lcd.setCursor(0, 1);
        lcd.print("Tente novamente");
      }
      else
      {
        lcd.setCursor(0, 0);
        lcd.print("ACESSO BLOQUEADO");
        lcd.setCursor(0, 1);
        lcd.print("Sem cadastro");
      }
    }
    else if (DB_response == DB_ALLOWED)
    {

      step_AccessManager = STEP_ALLOWED;
      reset_Sensors();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACESSO LIBERADO");
    }

    break;

  case STEP_CHECK_DB_DATETIME:
    DB_response = get_DBresponse();

    if (DB_response == DB_ALLOWED)
    {
      reset_Sensors();
      step_AccessManager = STEP_ALLOWED;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACESSO LIBERADO");
    }
    else if (DB_response == DB_NOT_ALLOWED)
    {
      step_AccessManager = STEP_NOT_ALLOWED;
      timer_error_accessManager = millis();
      lcd.clear();

      if (errorDB == OVERFLOW_ERROR)
      {
        lcd.setCursor(0, 0);
        lcd.print("Servidor Offline");
        lcd.setCursor(0, 1);
        lcd.print("Tente novamente");
      }
      else
      {
        lcd.setCursor(0, 0);
        lcd.print("ACESSO BLOQUEADO");
        lcd.setCursor(0, 1);
        if (errorDB == TIME_ERROR)
        {
          lcd.print("Fora do Horario");
        }
        else if (errorDB == DATE_ERROR)
        {
          lcd.print("Fora do Periodo");
        }
      }
    }

    break;

  case STEP_ALLOWED:

    if (solenoidLogic != NORMALLY_RELEASED)
    {
      solenoid = 1;
    }

    ok_Led = 1;
    errorLed = 0;

    if (read_Sensors() == MOVEMENT_OVERFLOW)
    {

      step_AccessManager = STEP_NOT_ALLOWED;
      timer_error_accessManager = millis();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACESSO BLOQUEADO");
      lcd.setCursor(0, 1);
      lcd.print("Tempo excedido");
    }
    else if (read_Sensors() == MOVEMENT_COMPLETED)
    {
      solenoid = 0;
      ok_Led = 0;
      errorLed = 0;
      step_AccessManager = STEP_REGISTER_DIRECTION;

      flagDelayAllowed = true;
      timerDelayAllowed = millis();
    }

    break;

  case STEP_REGISTER_DIRECTION:
    set_DB_last_access(get_directionAccess());

    DB_response = get_DBresponse();

    if ((DB_response == DB_ACCESS_REGISTERED) || (DB_response == DB_ACCESS_NOT_REGISTERED))
    {
      step_AccessManager = STEP_INITIAL;
    }

    break;

  case STEP_NOT_ALLOWED:
    if ((millis() - timer_error_accessManager) >= error_time)
    {
      solenoid = 0;
      ok_Led = 0;
      errorLed = 0;
      step_AccessManager = STEP_INITIAL;
      set_DB_target(0);
      reset_Sensors();
      solenoidBlockedByMovement = false;
    }
    else
    {
      if (solenoidBlockedByMovement == true)
      {
        lcd.setCursor(0, 0);
        lcd.print("ACESSO BLOQUEADO");
        lcd.setCursor(0, 1);
        lcd.print("Cracha nao lido!");
        solenoid = 1;
      }
      else
      {
        solenoid = 0;
      }

      
      ok_Led = 0;
      errorLed = 1;
    }

    break;
  }
  task_DBconnection();

  //FOR DIFFERENT SOLENOIDS TYPE
  if (solenoidLogic == NORMALLY_RELEASED)
  {
    if ((detectMovement() == true) && (step_AccessManager == STEP_STANDBY) && (solenoidBlockedByMovement == false) && (flagDelayAllowed == false))
    {
      step_AccessManager = STEP_NOT_ALLOWED;
      solenoidBlockedByMovement = true;
      timer_error_accessManager = millis();
    }
  }

  if(flagDelayAllowed == true)
  {
    if( (millis() - timerDelayAllowed) > 2000)
    {
      flagDelayAllowed = false;
    }

  }

  
}
