//vd - gnd, br/az - vcc, az - pin2(data,d-), br/vd pin3(clock,d+)

long int code_reader() {

  String st_code = String();
  long int code = 0;


  if ( (Serial1.available() > 0) && (!bufferRead) ) {
    bufferRead = 1;
    timer_Data = millis();
    Serial.println("Detectado");
  }
  else if ( (bufferRead) && ( (millis() - timer_Data) >= wait_receiveData)  ) {
    bufferRead = 0;
    numberChar = Serial1.available();

    for (int i = 0; i < numberChar; i++) {
      buffer[i] = Serial1.read();
      st_code.concat(buffer[i]);
      buffer[i] = '\0';
    }

    code = st_code.toInt();
  }
  //Serial.println("Leitor:");
  //Serial.println(code);

  return code;

}
