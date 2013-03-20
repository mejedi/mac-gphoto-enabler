#! /bin/bash

# anti-sudo
if [ "`id -nu`" = root ]; then
  echo "Running with scissors is sort of dangerous: do not use sudo" 1>&2
  exit -1
fi

dbfile="/Library/Caches/com.apple.ImageCaptureNotifications.DeviceDiscoveryDatabase.$(id -u)"

if [ ! -f "$dbfile" ]; then
  echo "File not found: $dbfile" 1>&2
  exit -1
fi

version="$(sqlite3 "$dbfile" 'SELECT typeID,value from DBVersion')"
if [ $? -ne 0 ]; then
  exit -1
fi

# version check
case "$version" in

  '1684174450|1')
    ;;

  *)
    echo "Unknown database version $version" 1>&2
    exit -1;;
esac

#
# function dispatch
#
case "$0" in

  # disable
  */gphoto-disable.sh)
    sqlite3 "$dbfile" "\
BEGIN TRANSACTION;\
INSERT INTO IOUSBInterface SELECT * FROM _IOUSBInterface;\
DROP TABLE _IOUSBInterface;\
COMMIT TRANSACTION;" || (
      if [ -z "$(sqlite3 "$dbfile" ".schema _IOUSBInterface" 2>/dev/null)" ]; then
        echo "Not enabled yet?" 1>&2;
        prompt="Reset anyway?"
      else
        echo "Something is wrong" 1>&2;
        prompt="Reset to a well known state?"
      fi
        
        read -p "$prompt ";
        while [[ ! "$REPLY" =~ ^[yYnN]$ ]]; do
          read -p "[y|n] ";
        done
        if [[ "$REPLY" =~ ^[yY]$ ]]; then
          rm "$dbfile" && (echo "Please end your session to complete restore" 1>&2);
          exit 0;
        else
          exit -1;
        fi
    )
    exit 0;;

  # enable
  *)
    schema=$(sqlite3 "$dbfile" ".schema IOUSBInterface")
    if [ $? -ne 0 ]; then
      exit -1;
    fi
    patch_trn="\
BEGIN TRANSACTION;\
$(echo "$schema" | sed -e s/IOUSBInterface/_IOUSBInterface/)\
INSERT INTO _IOUSBInterface SELECT * FROM IOUSBInterface WHERE bInterfaceClass==6 AND bInterfaceSubClass==1 AND bInterfaceProtocol==1;\
DELETE FROM IOUSBInterface WHERE ID in (SELECT ID FROM _IOUSBInterface);\
COMMIT TRANSACTION;"
    sqlite3 "$dbfile" "$patch_trn" || (
      if [ -n "$(sqlite3 "$dbfile" ".schema _IOUSBInterface" 2>/dev/null)" ]; then
        echo "Already enabled?" 1>&2;
        echo "Try gphoto-disable.sh to clean the mess" 1>&2;
      fi
      exit -1;
    )
    exit 0;;
esac

