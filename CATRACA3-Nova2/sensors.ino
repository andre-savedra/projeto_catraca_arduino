//#include "CATRACA3-Nova.ino"

byte read_Sensors() {

  if( (sensor1 != SENSOR_HOME_POSITION) && (sensor2 == SENSOR_HOME_POSITION) && (!step_sensors) ) {
    directionAccess = ENTER;
    step_sensors = MOVEMENT_STARTED;
    timer_movement = millis();
  }
  else if ( (sensor1 == SENSOR_HOME_POSITION) && (sensor2 != SENSOR_HOME_POSITION) && (!step_sensors) ) {
    directionAccess = EXIT;
    step_sensors = MOVEMENT_STARTED;
    timer_movement = millis();
  }

  if ( (millis() - timer_movement) >= maxTime_Movement ) {
    step_sensors = MOVEMENT_OVERFLOW;
  }
  else {

    if ((sensor1 != SENSOR_HOME_POSITION) && (sensor2 != SENSOR_HOME_POSITION) ) {
      buffer_sensors = 1;
    }

    if( (buffer_sensors) && (sensor1 == SENSOR_HOME_POSITION) && (sensor2 == SENSOR_HOME_POSITION) ) {
      buffer_sensors = 0;
      step_sensors = MOVEMENT_COMPLETED;
    }
  }


  return step_sensors;
}

void reset_Sensors() {
  directionAccess = 0;
  buffer_sensors = 0;
  step_sensors = 0;
  timer_movement = millis();
}

byte get_directionAccess() {
  return directionAccess;
}

boolean detectMovement(){

  if( ( sensor1 != oldSensor1 ) || ( sensor2 != oldSensor2 ) )
  {
    oldSensor1 = sensor1;
    oldSensor2 = sensor2;
    Serial.println("sensor movimentado!");
    return true;
  }
  else{
    return false;
  }

}
