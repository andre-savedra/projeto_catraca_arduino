int DB_search(boolean enable, char *query, long int toFind, long int toFind2) {
  MySQL_Cursor * sql2 = new MySQL_Cursor(&db_connection);
  char db_command[250];
  long selected = NOTHING;
  unsigned long timer_functionSelect = 0;


  if (!enable) {
    row_values *row = NULL;
    selected = NOTHING;

    delete sql2;
  }
  else {

    if (!toFind2) {
      sprintf(db_command, query, toFind);
    }
    else {
      sprintf(db_command, query, toFind, toFind2);
    }



    sql2->execute(db_command);
    column_names *columns = sql2->get_columns();

    timer_functionSelect = millis();

    do {
      row = sql2->get_next_row();
      if (row != NULL) {
        selected = atol(row->values[0]);
      }
    } while ((row != NULL) && ( (millis() - timer_functionSelect) <= MAX_TIME_SELECT) );

    delete sql2;
    //Serial.println(selected);

  }

  return selected;
}

String DB_searchStr(boolean enable, char *query, long int toFind, long int toFind2) {
  MySQL_Cursor * sql2 = new MySQL_Cursor(&db_connection);
  char db_command[250];
  String selected = "NOTHING";
  unsigned long timer_functionSelect = 0;


  if (!enable) {
    row_values *row = NULL;
    selected = "NOTHING";

    delete sql2;
  }
  else {

    if (!toFind2) {
      sprintf(db_command, query, toFind);
    }
    else {
      sprintf(db_command, query, toFind, toFind2);
    }

    sql2->execute(db_command);
    column_names *columns = sql2->get_columns();

    timer_functionSelect = millis();

    do {
      row = sql2->get_next_row();
      if (row != NULL) {
        selected = row->values[0];
      }
    } while ((row != NULL) && ( (millis() - timer_functionSelect) <= MAX_TIME_SELECT) );

    delete sql2;
    //Serial.println(selected);

  }

  return selected;
}
