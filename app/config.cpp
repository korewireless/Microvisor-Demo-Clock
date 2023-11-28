/*
 * Microvisor Clock Demo -- Config namespace
 *
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#include "main.h"


using std::vector;
using std::string;


#ifdef __cplusplus
extern "C" {
#endif
// Required on STM32 HAL callouts implemented in C++
void TIM8_BRK_IRQHandler(void);
#ifdef __cplusplus
}
#endif


// Central store for notification records.
// Holds SHARED_NC_BUFFER_SIZE_R records at a time -- each record is 16 bytes in size.
static volatile MvNotification  notificationCenter[SHARED_NC_BUFFER_SIZE_R] __attribute__((aligned(8)));
static volatile uint32_t        notificationIndex = 0;
static          Handles         handles = { 0, 0, 0 };
       volatile bool            receivedConfig = false;



namespace Config {

bool getPrefs(Prefs& prefs) {

    // Check for a valid channel handle
    if (!Channel::open()) return false;

    // Set up the request parameters
    MvConfigKeyToFetch keyOne;
    keyOne.scope = MV_CONFIGKEYFETCHSCOPE_DEVICE;     // A device-level value
    keyOne.store = MV_CONFIGKEYFETCHSTORE_CONFIG;     // A config-type value
    keyOne.key = {
        .data = (uint8_t*)"prefs",
        .length = 5
    };

    const uint32_t itemCount = 1;
    MvConfigKeyToFetch keys[itemCount];
    keys[0] = keyOne;

    MvConfigKeyFetchParams request;
    request.num_items = itemCount;
    request.keys_to_fetch = keys;

    enum MvStatus status = mvSendConfigFetchRequest(handles.channel, &request);
    if (status != MV_STATUS_OKAY) {
        server_error("Could not issue config fetch request");
        Channel::close();
        return false;
    }

    // Wait for the data to arrive
    server_log("Awaiting params...");
    receivedConfig = false;
    const uint32_t startTick = HAL_GetTick();

    while (true) {
        // Break out after timeout or successful config retrieval
        if (receivedConfig || (HAL_GetTick() - startTick > CONFIG_WAIT_PERIOD_MS)) break;
        __asm("nop");
    }

    if (!receivedConfig) {
        // Request timed out
        server_error("Config fetch request timed out");
        Channel::close();
        return false;
    }

    // Parse the received data record
    server_log("Received params");
    MvConfigResponseData response;
    response.result = MV_CONFIGFETCHRESULT_OK;
    response.num_items = 0;

    status = mvReadConfigFetchResponseData(handles.channel, &response);
    if (status != MV_STATUS_OKAY || response.result != MV_CONFIGFETCHRESULT_OK || response.num_items != itemCount) {
        if (response.result != MV_CONFIGFETCHRESULT_OK || response.num_items != itemCount) {
            server_error("Please set your config as detailed in the Read Me file");
        } else {
            server_error("Could not get config item (status: %i; result: %i)", status, response.result);
        }

        Channel::close();
        return false;
    }

    // Map the extent of `value` to the bytesize of your JSON
    uint8_t value[257] = {0};
    uint32_t valueLength = 0;
    enum MvConfigKeyFetchResult result = MV_CONFIGKEYFETCHRESULT_OK;

    MvConfigResponseReadItemParams item;
    item.item_index = 0;
    item.result = &result;
    item.buf = {
        .data = &value[0],
        .size = 256,
        .length = &valueLength
    };

    // Get the value itself
    status = mvReadConfigResponseItem(handles.channel, &item);
    if (status != MV_STATUS_OKAY || result != MV_CONFIGKEYFETCHRESULT_OK) {
        server_error("Could not get config item (status: %i; result: %i)", status, result);
        Channel::close();
        return false;
    }

    server_log("Received: %s", value);

    // Apple the settings input to the prefs structure
    // If a key is absent from the JSON, the cast value
    // defaults to zero/false.
    DynamicJsonDocument settings(256);
    DeserializationError err = deserializeJson(settings, value);
    if (err == DeserializationError::Ok) {
        prefs.mode          = (bool)settings["mode"];
        prefs.bst           = (bool)settings["bst"];
        prefs.colon         = (bool)settings["colon"];
        prefs.flash         = (bool)settings["flash"];
        prefs.brightness    = (uint32_t)settings["brightness"];
        prefs.led           = (bool)settings["led"];
    }

    Channel::close();
    return true;
}


namespace Channel {

/**
 * @brief Open a new config fetch channel.
 *
 * @returns `true` if the channel is open, otherwise `false`.
 */
bool open(void) {

    // Set up the HTTP channel's multi-use send and receive buffers
    static volatile uint8_t configRxBuffer[CONFIG_RX_BUFFER_SIZE_B] __attribute__((aligned(512)));
    static volatile uint8_t configTxBuffer[CONFIG_TX_BUFFER_SIZE_B] __attribute__((aligned(512)));

    if (handles.channel == 0) {
        // No network connection yet? Then establish one
        Network::open();
        if (handles.network == 0) return false;

        // Get the network channel handle.
        // NOTE This is set in `logging.c` which puts the network in place
        //      (ie. so the network handle != 0) well in advance of this being called
        // Configure the required data channel
        MvOpenChannelParams channelConfig;
        channelConfig.version = 1;
        channelConfig.v1 = {
            .notification_handle = handles.notification,
            .notification_tag    = USER_TAG_CONFIG_OPEN_CHANNEL,
            .network_handle      = handles.network,
            .receive_buffer      = (uint8_t*)configRxBuffer,
            .receive_buffer_len  = sizeof(configRxBuffer),
            .send_buffer         = (uint8_t*)configTxBuffer,
            .send_buffer_len     = sizeof(configTxBuffer),
            .channel_type        = MV_CHANNELTYPE_CONFIGFETCH,
            .endpoint            = {
                .data = (uint8_t*)"",
                .length = 0
            }
        };

        // Ask Microvisor to open the channel
        // and confirm that it has accepted the request
        enum MvStatus status = mvOpenChannel(&channelConfig, &handles.channel);
        if (status != MV_STATUS_OKAY) {
            server_error("Could not open config channel. Status: %lu", status);
            return false;
        }
    }

    server_log("Config Channel handle: %lu", handles.channel);
    return true;
}


/**
 * @brief Close the currently open HTTP channel.
 */
void close(void) {

    // If we have a valid channel handle -- ie. it is non-zero --
    // then ask Microvisor to close it and confirm acceptance of
    // the closure request.
    if (handles.channel != 0) {
        const MvChannelHandle oldHandle = handles.channel;
        enum MvStatus status = mvCloseChannel(&handles.channel);
        if (status == MV_STATUS_OKAY) {
            server_log("Config Channel closed (handle %lu)", oldHandle);
        } else {
            server_error("Could not close Config Channel");
        }
    }
}


}   // Namespace Channel


namespace Network {

void open(void) {

    // Configure the network's notification center,
    // but bail if it fails
    if (!setupNotificationCenter()) return;

    // Check if we need to establish a network
    if (handles.network == 0) {
        // Configure the network connection request
        MvRequestNetworkParams networkConfig;
        networkConfig.version = 1;
        networkConfig.v1 = {
            .notification_handle = handles.notification,
            .notification_tag = USER_TAG_LOGGING_REQUEST_NETWORK,
        };

        // Ask Microvisor to establish the network connection
        // and confirm that it has accepted the request
        mvRequestNetwork(&networkConfig, &handles.network);

        // The network connection is established by Microvisor asynchronously,
        // so we wait for it to come up before opening the data channel -- which
        // would fail otherwise
        enum MvNetworkStatus netStatus;
        while (1) {
            // Request the status of the network connection, identified by its handle.
            // If we're good to continue, break out of the loop...
            if (mvGetNetworkStatus(handles.network, &netStatus) == MV_STATUS_OKAY && netStatus == MV_NETWORKSTATUS_CONNECTED) {
                break;
            }

            // ... or wait a short period before retrying
            for (volatile unsigned i = 0; i < 50000; ++i) {
                // No op
                __asm("nop");
            }
        }
    }

    server_log("Network handle: %lu", handles.network);
}


uint32_t getState(void) {

    if (handles.network == 0) return OFFLINE;
    MvNetworkStatus netStatus;
    MvStatus status = mvGetNetworkStatus(handles.network, &netStatus);
    if (status != MV_STATUS_OKAY) return UNKNOWN;
    return netStatus;
}


/**
 * @brief Configure the network Notification Center.
 */
bool setupNotificationCenter(void) {

    if (handles.notification == 0) {
        // Clear the notification store
        memset((void *)notificationCenter, 0xff, sizeof(notificationCenter));

        static struct MvNotificationSetup notificationConfig = {
            .irq = TIM8_BRK_IRQn,
            .buffer = (struct MvNotification *)notificationCenter,
            .buffer_size = sizeof(notificationCenter)
        };

        // Ask Microvisor to establish the notification center
        // and confirm that it has accepted the request
        enum MvStatus status = mvSetupNotifications(&notificationConfig, &handles.notification);
        if (status == MV_STATUS_OKAY) {
            // Start the notification IRQ
            NVIC_ClearPendingIRQ(TIM8_BRK_IRQn);
            NVIC_EnableIRQ(TIM8_BRK_IRQn);
        } else {
            handles.notification = 0;
            return false;
        }
    }

    server_log("Notification Center handle: %lu", handles.notification);
    return true;
}


}   // Namespace Network


}   // Namespace Config


/**
 * @brief The shared channel notification interrupt handler.
 *
 * This is called by Microvisor. We need to check for key events,
 * and signal the app via flags that data is available. This center
 * handles notifications from two channels, for config fetches and
 * HTTP requests, so we use tags to determine the source channel.
 */
void TIM8_BRK_IRQHandler(void) {

    // Check for readable data in the HTTP channel
    //HAL_GPIO_WritePin(LED_GPIO_BANK, LED_GPIO_PIN, GPIO_PIN_SET);
    bool gotNotification = false;
    volatile MvNotification& notification = notificationCenter[notificationIndex];
    switch(notification.tag) {
        // Config fetch channel notifications
        case USER_TAG_CONFIG_OPEN_CHANNEL:
            if (notification.event_type == MV_EVENTTYPE_CHANNELDATAREADABLE) {
                // Flag we need to access received data and to close the channel
                // when we're back in the main loop. This lets us exit the ISR quickly.
                // Do NOT make Microvisor System Calls in the ISR!
                receivedConfig = true;
                gotNotification = true;
            }

            break;
        case USER_TAG_LOGGING_REQUEST_NETWORK:
            if (notification.event_type == MV_EVENTTYPE_NETWORKSTATUSCHANGED) {
                // Change in network status -- don't really need this,
                // but it clears the notification
                gotNotification = true;
            }

            break;
        // Others
        default:
            break;
    }

    // We had a relevant notification
    if (gotNotification) {
        // Point to the next record to be written
        notificationIndex = (notificationIndex + 1) % SHARED_NC_BUFFER_SIZE_R;

        // Clear the current notifications event
        // See https://www.twilio.com/docs/iot/microvisor/microvisor-notifications#buffer-overruns
        notification.event_type = (MvEventType)0;
    }
}

/*
#define     USER_TAG_LOGGING_REQUEST_NETWORK    1
#define     USER_TAG_LOGGING_OPEN_CHANNEL       2
#define     USER_TAG_HTTP_OPEN_CHANNEL          3
#define     USER_TAG_CONFIG_OPEN_CHANNEL        4
*/
