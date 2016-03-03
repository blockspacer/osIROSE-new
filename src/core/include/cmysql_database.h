#ifndef _CMYSQL_DATABASE_H_
#define _CMYSQL_DATABASE_H_

#include "idatabase.h"
#include <mysql++.h>
#include "logconsole.h"
#include <mutex>

namespace Core {
class CMySQL_Result : public IResult {
 public:
  CMySQL_Result(mysqlpp::StoreQueryResult);
  virtual ~CMySQL_Result() {}

  virtual bool incrementRow();
  virtual uint32_t	size() const {return res_.size();}

  virtual bool getString(std::string const &, std::string &data);
  virtual bool getInt(std::string const &, uint32_t &data);
  virtual bool getFloat(std::string const &, float &data);

 private:
  mysqlpp::StoreQueryResult res_;

  template <typename T>
  bool getData(std::string const &name, T &data) {
    auto tmp = res_[row_][name.c_str()];
    if (tmp.is_null()) return false;
    data = static_cast<T>(tmp);
    return true;
  }
};

class CMySQL_Database : public IDatabase {
 public:
  CMySQL_Database();
  CMySQL_Database(std::string _host, std::string _database, std::string _user,
                  std::string _password);
  virtual ~CMySQL_Database();

  virtual void Connect(std::string _host, std::string _database,
                       std::string _user, std::string _password);

  virtual void QExecute(std::string _query);
  virtual std::unique_ptr<IResult> QStore(std::string _query);

 private:
  std::string hostname_;
  std::string database_;
  std::string username_;
  std::string password_;

  std::mutex mutex_;
  mysqlpp::Connection conn_;
  CLogConsole log_;
  bool connected_;
};
}

#endif
