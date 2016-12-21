#include<stdio.h>
#include<stdlib.h>
#include<sqlite3.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
  int i;
  for(i = 0; i < argc; i++)
  {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

  }
  printf("\n");
  return 0;
}

int main()
{
    sqlite3 *db;
    int rc;
    char *Zerrmsg, *sql;

    rc = sqlite3_open("Log.db", &db);
    if(rc)
    {
        fprintf(stderr, "Database could not be opened Successfully: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    else
    {
        fprintf(stderr, "Database opened Successfully\n");
    }

    sql = "CREATE TABLE sender_receiver(" \
             "ID INT PRIMARY KEY, " \
             "SENDER_IP TEXT NOT NULL, " \
             "RECEIVER_IP TEXT NOT NULL, " \
             "MESSAGE TEXT );";
   rc = sqlite3_exec(db , sql, callback, 0,  &Zerrmsg);
   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "SQL Error: %s\n", Zerrmsg);
   }
   else
   {
      fprintf(stderr, "Table Inserted Successfully\n");
   }
   sqlite3_close(db);
   return 0;
}
