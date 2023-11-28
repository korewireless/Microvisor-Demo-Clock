/*
 * Microvisor Clock Demo -- Config namespace
 *
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _CONFIG_HEADER_
#define _CONFIG_HEADER_


/*
 * CONSTANTS
 */
#define     SHARED_NC_BUFFER_SIZE_R             16
#define     CONFIG_WAIT_PERIOD_MS               4000
#define     CONFIG_RX_BUFFER_SIZE_B             512
#define     CONFIG_TX_BUFFER_SIZE_B             512

#define     USER_TAG_LOGGING_REQUEST_NETWORK    1
#define     USER_TAG_LOGGING_OPEN_CHANNEL       2
#define     USER_TAG_HTTP_OPEN_CHANNEL          3
#define     USER_TAG_CONFIG_OPEN_CHANNEL        4


/*
 * STRUCTURES
 */
typedef struct {
    MvNotificationHandle notification;
    MvNetworkHandle      network;
    MvChannelHandle      channel;
} Handles;


enum {
    OFFLINE = 0,
    ONLINE = 1,
    CONNECTING = 2,
    UNKNOWN = 99
};


/*
 * PROTOTYPES
 */
namespace Config {

    namespace Channel {
        bool        open(void);
        void        close(void);
    }

    namespace Network {
        void        open(void);
        bool        setupNotificationCenter(void);
        uint32_t    getState(void);
    }

    bool    getPrefs(Prefs& prefs);
}


#endif      // _CONFIG_HEADER_
