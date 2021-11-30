
byte read_Sensors() {

  if (sensor1 && !sensor2 && !step_sensors) {
    directionAccess = ENTER;
    step_sensors = MOVEMENT_STARTED;
    timer_movement = millis();
  }
  else if (!sensor1 && sensor2 && !step_sensors) {
    directionAccess = EXIT;
    step_sensors = MOVEMENT_STARTED;
    timer_movement = millis();
  }

  if ( (millis() - timer_movement) >= maxTime_Movement ) {
    step_sensors = MOVEMENT_OVERFLOW;
  }
  else {

    if (sensor1 && sensor2) {
      buffer_sensors = 1;
    }

    if (buffer_sensors && !sensor1 && !sensor2) {
      buffer_sensors = 0;
      step_sensors = MOVEMENT_COMPLETED;
    }
  }

/*
  if(sensor1){
    buffer_sensors = 1;
    directionAccess = ENTER;
    step_sensors = MOVEMENT_STARTED;
  }

  if(!sensor1 && buffer_sensors){
    buffer_sensors = 0;
    step_sensors = MOVEMENT_COMPLETED;
  }
*/



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
