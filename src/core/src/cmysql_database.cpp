#include <string>
#include "cmysql_database.h"
#include <exception>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace Core {

bool CMySQL_Row::getString(std::string const &name, std::string &data) {
  return getData<std::string>(name, data);
}

bool CMySQL_Row::getInt(std::string const &name, uint32_t &data) {
  return getData<uint32_t>(name, data);
}

bool CMySQL_Row::getFloat(std::string const &name, float &data) {
  return getData<float>(name, data);
}

CMySQL_Result::CMySQL_Result(const mysqlpp::StoreQueryResult &_res)
    : IResult() {
	for (auto const &it : _res)
		rows_.emplace_back(std::unique_ptr<IRow>(new CMySQL_Row(it)));
}

bool CMySQL_Result::incrementRow() {
	uint32_t	tmp = current_row_;
	current_row_ = current_row_ >= rows_.size() ? rows_.size() - 1 : current_row_ + 1;
	return tmp == rows_.size();
}

bool CMySQL_Result::getString(std::string const &name, std::string &data) {
  return rows_[current_row_]->getString(name, data);
}

bool CMySQL_Result::getInt(std::string const &name, uint32_t &data) {
  return rows_[current_row_]->getInt(name, data);
}

bool CMySQL_Result::getFloat(std::string const &name, float &data) {
  return rows_[current_row_]->getFloat(name, data);
}

CMySQL_Database::CMySQL_Database()
    : hostname_(""),
      database_(""),
      username_(""),
      password_(""),
      connected_(false) {
  logger_ = spdlog::get( "console" );
  if (logger_ == nullptr)
    logger_ = spdlog::stdout_logger_mt( "console" );
}

CMySQL_Database::CMySQL_Database(std::string _host, std::string _database,
                                 std::string _user, std::string _password)
    : hostname_(_host),
      database_(_database),
      username_(_user),
      password_(_password),
      connected_(false) {
  logger_ = spdlog::get( "console" );
  if (logger_ == nullptr)
    logger_ = spdlog::stdout_logger_mt( "console" );
  try {
    conn_.connect(database_.c_str(), hostname_.c_str(), username_.c_str(),
                 password_.c_str());
    logger_->notice() << "Connected to database";
  } catch (const std::exception &e) {
    logger_->critical() << Color::FG_RED << "Error while connecting to the database: " << conn_.error() << Color::CL_RESET;
    throw e;
  }
  connected_ = true;
}

void CMySQL_Database::Connect(std::string _host, std::string _database,
                              std::string _user, std::string _password) {
  hostname_ = (_host);
  database_ = (_database);
  username_ = (_user);
  password_ = (_password);
  connected_ = false;

  try {
    conn_.connect(database_.c_str(), hostname_.c_str(), username_.c_str(),
                 password_.c_str());
  } catch (const std::exception &e) {
    logger_->critical() << Color::FG_RED << "Error while connecting to the database: " << conn_.error() << Color::CL_RESET;
    throw e;
  }
  connected_ = true;
}

std::unique_ptr<IResult> CMySQL_Database::QStore(std::string _query) {
  if (!connected_) {
    logger_->critical() << Color::FG_RED << "Error while executing the query: " << _query.c_str() << ": not connected" << Color::CL_RESET;
    throw std::runtime_error( "Error not connected" );
  }
  std::lock_guard<std::mutex> lock(mutex_);
  conn_.thread_start(); // This is if we pass the database around different threads
  logger_->debug() << "Executing query: " << _query.c_str();
  mysqlpp::Query query = conn_.query(_query.c_str());
  return std::unique_ptr<IResult>(new CMySQL_Result(query.store()));
}

void CMySQL_Database::QExecute(std::string _query) {
  if (!connected_) {
    logger_->critical() << Color::FG_RED << "Error while executing the query: " << _query.c_str() << ": not connected" << Color::CL_RESET;
    throw std::runtime_error( "Error not connected" );
  }
  std::lock_guard<std::mutex> lock(mutex_);
  conn_.thread_start(); // This is if we pass the database around different threads
  logger_->debug() << "Executing query: " << _query.c_str();
  auto query = conn_.query(_query.c_str());
  query.exec(_query.c_str());
}
}