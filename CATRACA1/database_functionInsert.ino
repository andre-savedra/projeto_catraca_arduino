boolean DB_insert(boolean enable, char *query, long int toFind) {
  MySQL_Cursor * sql = new MySQL_Cursor(&db_connection);
  char db_command[250];

  if (!enable) {
    stepInsert = 0;
    delete sql;
  }

  else {
    sprintf(db_command, query, toFind);

    sql->execute(db_command);
    stepInsert = 1;
    delete sql;

  }
  return stepInsert;



}
