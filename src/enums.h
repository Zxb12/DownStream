#ifndef ENUMS_H
#define ENUMS_H

#define APP_NAME            "DownStream"
#define APP_ORGANIZATION    "Camille Maller"
#define VERSION             "1.5"
#define VERSION_NBR         6

#define VERSION_HOST        "supversion.frugebul.tk"

#define DOWNLOAD_BUFFER 64 * 1024  //Le buffer doit être plus grand que la page d'erreur renvoyée par Megaupload (limit exceeded...)
#define DOWNLOAD_NO_RECV_TIMER  15
#define DOWNLOAD_LIMIT_EXCEEDED     "<BODY>Download limit exceeded</BODY></HTML>"
#define FILE_TEMPORARILY_UNAVAILABLE  "The file you are trying to access is temporarily unavailable. Please try again later."
#define RETRY_TIMER 10
#define LIMIT_REACHED_TIMER 60

#define BEFORE_LINK_AVAILABLE   "secondes d'attente"
#define BEFORE_NEXT_TRY         "secondes avant le prochain essai"

#define INFO_EXTRACTION_TIMEOUT         15 * IN_MILLISECONDS
#define DOWNLOAD_SPEED_UPDATE_INTERVAL  1 * IN_MILLISECONDS
#define DOWNLOAD_SPEED_AVERAGE_TIME     5 * IN_MILLISECONDS
#define IN_MILLISECONDS 1000

#define MEGAUPLOAD QUrl("http://www.megaupload.com")

#include "log.h"

enum AuthLevel
{
    GUEST = 45, //secondes d'attente
    USER = 25,
    PREMIUM = 0,
};

enum DownloadState
{
    IDLE,
    AUTH,
    DOWNLOADING,
    RECONNECTING,
};

enum AuthError
{
    AUTH_INVALID_LOGIN,
    AUTH_CONNECTION_REFUSED,
    AUTH_REMOTE_HOST_CLOSED,
    AUTH_HOST_NOT_FOUND,
    AUTH_TEMP_NETWORK_FAILURE,
    AUTH_NETWORK_ERROR,
    AUTH_PROTOCOL_FAILURE,
    AUTH_UNDEFINED_ERROR,
};

enum DownloadError
{
    LINK_NETWORK_ERROR,
    LINK_NOT_FOUND,
    FILE_COULD_NOT_BE_OPENED,
    FILE_CORRUPT_RESTART_DOWNLOAD,
    DOWNLOAD_LIMIT_REACHED,
    DOWNLOAD_NETWORK_ERROR,
    DOWNLOAD_EMPTY,
    DOWNLOAD_SOCKET_TIMEOUT,
};

#endif // ENUMS_H
