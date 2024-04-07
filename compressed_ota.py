import gzip
import shutil
import os
Import("env")

def compressFirmware(source, target, env):
    """ Compress ESP8266 firmware using gzip for 'compressed OTA upload' """
    SOURCE_FILE = env.subst("$BUILD_DIR") + os.sep + env.subst("$PROGNAME") + ".bin"
    COMPRESSED_FILE = SOURCE_FILE + '.gz'
    BACKUP_FILE = SOURCE_FILE + '.bak'

    # Kopiere die Originaldatei als Backup
    shutil.copy(SOURCE_FILE, BACKUP_FILE)

    if os.path.exists(SOURCE_FILE):
        print("Compressing firmware for upload...")
        with open(SOURCE_FILE, 'rb') as f_in:
            with gzip.open(COMPRESSED_FILE, 'wb') as f_out:
                shutil.copyfileobj(f_in, f_out)

    if os.path.exists(COMPRESSED_FILE):
        ORG_FIRMWARE_SIZE = os.stat(SOURCE_FILE).st_size
        GZ_FIRMWARE_SIZE = os.stat(COMPRESSED_FILE).st_size

        print("Compression reduced firmware size by {:.0f}% (was {} bytes, now {} bytes)".format((GZ_FIRMWARE_SIZE / ORG_FIRMWARE_SIZE) * 100, ORG_FIRMWARE_SIZE, GZ_FIRMWARE_SIZE))

env.AddPreAction("upload", compressFirmware)
