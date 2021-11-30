
void task_ReadConfigs()
{

    int responseSelect = 0;

    switch (step_DB_Configs)
    {

    case DB_CONFIG_INITIAL:
        responseSelect = DB_search(0, '\0', 0, 0);
        step_DB_Configs = DB_CONFIG_READ_RELEASE;

        break;

    case DB_CONFIG_READ_RELEASE:
        responseSelect = DB_search(1, DB_SEARCH_RELEASE, 1, 0);
        if (responseSelect == 1)
        {
            myConfig.release = true;
        }
        else
        {
            myConfig.release = false;
        }

        step_DB_Configs = DB_CONFIG_READ_CONTROLTIME;
        break;

    case DB_CONFIG_READ_CONTROLTIME:
        responseSelect = DB_search(1, DB_SEARCH_TIME_CONTROL, 1, 0);
        if (responseSelect == 1)
        {
            myConfig.timeControl = true;
        }
        else
        {
            myConfig.timeControl = false;
        }

        step_DB_Configs = DB_CONFIG_READ_CONTROLDATE;

        break;

    case DB_CONFIG_READ_CONTROLDATE:
        responseSelect = DB_search(1, DB_SEARCH_DATE_CONTROL, 1, 0);
        if (responseSelect == 1)
        {
            myConfig.dateControl = true;
        }
        else
        {
            myConfig.dateControl = false;
        }

        step_DB_Configs = DB_CONFIG_RESET;
        break;

    case DB_CONFIG_RESET:
        responseSelect = DB_search(0, '\0', 0, 0);
        step_DB_Configs = DB_CONFIG_END;
        break;

    case DB_CONFIG_END:
    //do not do anything
        break;
    }
}
