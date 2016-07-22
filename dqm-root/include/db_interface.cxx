#define PORT 3306
#include <mysql/mysql.h>
#include <Python.h>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <array>
#include <memory>
#include <unordered_map>

using namespace std;


string stringFromChar(const char* queryResult)
{
  int qint = atoi(queryResult);
  return to_string((long long int)qint);
}


MYSQL* connectDB()
{
  MYSQL *Database;
  Database = mysql_init(0);
  if (mysql_real_connect(Database,"gem904daq01.cern.ch","gemdaq","gemdaq","ldqm_db",PORT,0,CLIENT_COMPRESS) == 0) {
    std::string message("Error connecting to database '");
    message += "' : ";
    message += mysql_error(Database);
    Database = 0;
    std::cout << message << std::endl;
  }
  return Database;
}



//Queries that return multiple values
vector<string> manyDBQuery(MYSQL * Database, std::string m_Query)
{
  vector<string> result;
  vector<string> null_return{"0"};
  const char * manyQuery = m_Query.c_str();

  int qresult = mysql_query(Database,manyQuery);
  if (qresult) {
    std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
    return null_return;
  }
  //else
  //  std::cout << "MySQL query success: " << manyQuery << std::endl;

  MYSQL_RES *res = mysql_use_result(Database);
  MYSQL_ROW row;
  
  while ((row=mysql_fetch_row(res)))
    {
      if (row[0]!=0) {
        string resstr(row[0]); 
        result.push_back(resstr);
      }
    }

  mysql_free_result(res);

  return result;
}

  
//Queries that should return a single value
char* simpleDBQuery(MYSQL * Database, std::string m_Query)
{
  char * null_return = "0";
  const char * query = m_Query.c_str();
  try {
    int rv = mysql_query(Database,query);
    if (rv) {
      std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
      return null_return;
    }
    else
      if(DEBUG) std::cout << "MySQL query success: " << m_Query << std::endl;
    MYSQL_RES* res = mysql_use_result(Database);
    MYSQL_ROW  row = mysql_fetch_row(res);
    if (row == 0) {
      std::cout << "Query result " << m_Query << " empty" << std::endl;
      return null_return;
    }
    char* retval = row[0];
    mysql_free_result(res);
    return retval;
  } catch (std::exception& e) {
    std::cout << "simpleDBQuery caught std::exception " << e.what() << std::endl;
  }
    
}

unsigned int getRunNumber(MYSQL* Database)
{
  
  std::string    setup = "teststand";
  std::string   period = "2016T";
  std::string location = "TIF";
  std::string lastRunNumberQuery = "SELECT Number FROM ldqm_db_run WHERE Station LIKE '";
  lastRunNumberQuery += location;
  lastRunNumberQuery += "' ORDER BY id DESC LIMIT 1;";
  const char * query = lastRunNumberQuery.c_str();
    
  try {
    int rv = mysql_query(Database,query);
    if (rv)
      std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
    else
      if(DEBUG) std::cout << "[getRunNumber]: MySQL query success: " << lastRunNumberQuery << std::endl;

    MYSQL_RES* res = mysql_use_result(Database);
    MYSQL_ROW  row = mysql_fetch_row(res);
    if (row == 0) {
      std::string errMsg = "[getRunNumber]: Query result " + lastRunNumberQuery + " empty";
      std::cout <<"[getRunNumber]: " << errMsg << std::endl;
    }
    unsigned int retval = strtoul(row[0],0,0);
    mysql_free_result(res);
      
    if (DEBUG) std::cout << "[getRunNumber]: New run number is: " << retval << std::endl;
    return retval;
  } catch (std::exception& e) {
    std::cout << "[getRunNumber]: caught std::exception " << e.what() << std::endl;
  }
}

unsigned int getChipIDFromID(MYSQL * Database, unsigned int db_id)
{
  std::string m_Query = "SELECT ChipID FROM ldqm_db_vfat WHERE id LIKE ";
  m_Query += std::to_string((long long int)db_id);
  const char * query = m_Query.c_str();
  try {
    int rv = mysql_query(Database,query);
    if (rv)
      std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
    else
      if(DEBUG) std::cout << "[getChipIDFromID]: MySQL query success: " << m_Query << std::endl;
    MYSQL_RES* res = mysql_use_result(Database);
    MYSQL_ROW  row = mysql_fetch_row(res);
    if (row == 0) {
      std::string errMsg = "[getChipIDFromID]: Query result " + m_Query + " empty";
      std::cout << errMsg << std::endl;
      return NULL;
    }
    unsigned int retval = strtoul(row[0],0,0);
    mysql_free_result(res);
      
    if (DEBUG) std::cout << "[getChipIDFromID]: ChipID is: " << std::hex << retval << std::dec << std::endl;
    return retval;
  } catch (std::exception& e) {
    std::cout << "[getChipIDFromID]: caught std::exception " << e.what() << std::endl;
  }
}
