#include "discovery_db_fixup.hh"

#include <limits.h>
#include <unistd.h>

#include <sqlite3.h>

#include <list>
#include <vector>
#include <string>
#include <stdexcept>

GPHOTO_ENABLER_NS_BEGIN

namespace {

  typedef std::list<std::vector<std::string>> sqlite3_exec_results;

  /*
   * Simple C++ wrapper for SQLite
   */
  class sqlite3_database: non_copyable {

  public:
    sqlite3_database(const std::string &db_file_path);
    ~sqlite3_database();
    sqlite3_exec_results exec(const std::string &sql_statements);

  private:
    static int exec_append_row_cb(void *, int, char **, char **);

    std::string              db_file_path;
    sqlite3                 *handle;
  };

}


std::unique_ptr<discovery_db_fixup> discovery_db_fixup::create() {
  char path_buf[PATH_MAX];

  snprintf(
    path_buf,
    sizeof path_buf,
    "/Library/Caches/com.apple.ImageCaptureNotifications.DeviceDiscoveryDatabase.%d",
    (int)getuid());

  try {
    return std::unique_ptr<discovery_db_fixup> (
      new discovery_db_fixup(path_buf));
  } catch (const std::exception &e) {

    fprintf(stderr, "Patching device discovery database: %s\n", e.what());
    return std::unique_ptr<discovery_db_fixup> ();
  }
}


discovery_db_fixup::discovery_db_fixup(const std::string &db_file_path_):
  db_file_path(db_file_path_) {

  fprintf(
    stderr, "patching device discovery database (%s)\n",
    db_file_path.c_str());

  sqlite3_database database (db_file_path);

  auto version = database.exec("SELECT typeID, value FROM DBVersion");

  if (version.size() != 1
    || version.front().at(0) != "1684174450"
    || version.front().at(1) != "1") {

    throw std::runtime_error("Version mismatch");
  }

  database.exec(
    "BEGIN TRANSACTION; "
    "CREATE TABLE IF NOT EXISTS _IOUSBInterface ("
      "ID integer primary key not null, typeID integer, "
      "bInterfaceClass integer, bInterfaceSubClass integer, "
      "bInterfaceProtocol integer); "
    "INSERT INTO _IOUSBInterface SELECT * FROM IOUSBInterface "
      "WHERE bInterfaceClass==6 AND bInterfaceSubClass==1 AND "
      "bInterfaceProtocol==1; "
    "DELETE FROM IOUSBInterface WHERE ID IN ("
      "SELECT ID FROM _IOUSBInterface); "
    "COMMIT TRANSACTION;");
}


discovery_db_fixup::~discovery_db_fixup() {

  fprintf(
    stderr,
    "reverting patch for device discovery database (%s)\n",
    db_file_path.c_str());

  try {

    sqlite3_database database (db_file_path);

    database.exec(
      "BEGIN TRANSACTION; "
      "INSERT INTO IOUSBInterface SELECT * FROM _IOUSBInterface; "
      "DROP TABLE _IOUSBInterface; "
      "COMMIT TRANSACTION;");

  } catch (const std::exception &e) {
    fprintf(stderr, "Restoring device discovery database: %s\n", e.what());
  }
}


sqlite3_database::sqlite3_database(const std::string &db_file_path_)
: db_file_path(db_file_path_){

  int status = sqlite3_open(db_file_path.c_str(), &handle);
  if (status != SQLITE_OK) {
    throw std::runtime_error(
      std::string("Error opening ")
      .append(db_file_path) /*
      .append(": ")
      .append(sqlite3_errstr(status))*/);
  }
}


sqlite3_database::~sqlite3_database() {
  sqlite3_close(handle);
}


sqlite3_exec_results sqlite3_database::exec(
  const std::string &sql_statements) {

  sqlite3_exec_results results;
  int status;
  struct guard_ {
    char *errmsg;
    guard_(): errmsg(NULL) { ; }
    ~guard_() { sqlite3_free(errmsg); }
  } guard;

  status = sqlite3_exec(
    handle,
    sql_statements.c_str(),
    exec_append_row_cb,
    &results,
    &guard.errmsg);

  if (status != SQLITE_OK) {
    throw std::runtime_error(
      std::string("SQLite error (db=")
      .append(db_file_path)
      .append("): ")
      .append(guard.errmsg));
  }

  return results;
}


int sqlite3_database::exec_append_row_cb(
  void *p, int col_count, char **col_values, char **col_names) {

  try {
    sqlite3_exec_results &results =
      *static_cast<sqlite3_exec_results *>(p);

    std::vector<std::string> row;
    row.reserve(col_count);

    for (int i = 0; i < col_count; i++) {
      row.push_back(col_values[i] ? col_values[i] : "");
    }

    results.push_back(std::move(row));

    return 0;
  } catch (const std::bad_alloc &) {
    return -1;
  }
}


GPHOTO_ENABLER_NS_END

