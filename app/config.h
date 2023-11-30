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
 * ENUMERATIONS
 */
enum class USER_TAG: uint32_t {
    LOGGING_REQUEST_NETWORK = 1,
    LOGGING_OPEN_CHANNEL,
    HTTP_OPEN_CHANNEL,
    CONFIG_OPEN_CHANNEL
};


/*
 * STRUCTURES
 */
typedef struct {
    MvNotificationHandle    notification;
    MvNetworkHandle         network;
    MvChannelHandle         channel;
} Handles;


/*
 * PROTOTYPES
 */
namespace Config {

    namespace Channel {
        bool                open(void);
        void                close(void);
    }

    namespace Network {
        void                open(void);
        bool                setupNotificationCenter(void);
        uint32_t            getState(void);
    }

    bool                    getPrefs(Prefs& prefs);
}


#endif      // _CONFIG_HEADER_
