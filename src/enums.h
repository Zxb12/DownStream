#ifndef ENUMS_H
#define ENUMS_H

#define APP_NAME            "DownStream"
#define APP_ORGANIZATION    "Camille Maller"
#define VERSION             "1.6"
#define VERSION_NBR         7

#define VERSION_HOST        "supversion.frugebul.tk"

#define DOWNLOAD_BUFFER               256 * 1024  //Le buffer doit �tre plus grand que la page d'erreur renvoy�e par Megaupload (limit exceeded...)
#define DOWNLOAD_NO_RECV_TIMER        60
#define DOWNLOAD_LIMIT_EXCEEDED       "<BODY>Download limit exceeded</BODY></HTML>"
#define PREMIUM_ACCOUNT_NEEDED        "<a href=\"?c=premium\" class=\"download_l_buy\"></a>"
#define FILE_TEMP_UNAVAILABLE_HINT    "The file you are trying to access is temporarily unavailable. Please try again later."
#define FILE_DELETED_HINT             "The file has been deleted because it was violating our Terms of service"
#define FILE_PASSWORD_PROTECTED_HINT  "The file you are trying to download is password protected"
#define RETRY_TIMER                   10
#define LIMIT_REACHED_TIMER           60

#define BEFORE_LINK_AVAILABLE   "secondes d'attente"
#define BEFORE_NEXT_TRY         "secondes avant le prochain essai"

#define INFO_EXTRACTION_TIMEOUT         15 * IN_MILLISECONDS
#define DOWNLOAD_SPEED_UPDATE_INTERVAL  1 * IN_MILLISECONDS
#define DOWNLOAD_SPEED_AVERAGE_TIME     5 * IN_MILLISECONDS
#define IN_MILLISECONDS                 1000
#define LINK_EXTRACTION_REGEXP          "http://www[0-9]*.megaupload.com/files/[^\"]*"
#define LINK_EXTRACTION_NEED_PASSWORD   "function postpassword()"

#define MEGAUPLOAD QUrl("http://www.megaupload.com")

#include "log.h"

enum AuthLevel
{
    GUEST = 15, //secondes d'attente
    USER = 10,
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
    PASSWORD_REQUIRED,
    NEED_PREMIUM,
    FILE_COULD_NOT_BE_OPENED,
    FILE_CORRUPT_RESTART_DOWNLOAD,
    DOWNLOAD_LIMIT_REACHED,
    DOWNLOAD_NETWORK_ERROR,
    DOWNLOAD_EMPTY,
    DOWNLOAD_SOCKET_TIMEOUT,
};

enum ExtractionError
{
    FILE_DELETED,
    PREMIUM_NEEDED,
    FILE_PASSWORD_PROTECTED,
    INVALID_DATA,
    NETWORK_ERROR,
    TIMEOUT_ERROR,
};

#endif // ENUMS_H
