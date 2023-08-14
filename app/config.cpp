/*
 * Config
 *
 * @version     1.0.0
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
static volatile MvNotification  notification_center[SHARED_NC_BUFFER_SIZE_R] __attribute__((aligned(8)));
static volatile uint32_t        notification_index = 0;
static          Handles         handles = { 0, 0, 0 };
       volatile bool            received_config = false;



namespace Config {

bool get_prefs(Prefs& prefs) {
    
    // Check for a valid channel handle
    if (!Channel::open()) return false;
    
    // Set up the request parameters
    MvConfigKeyToFetch key_one;
    key_one.scope = MV_CONFIGKEYFETCHSCOPE_DEVICE;     // A device-level value
    key_one.store = MV_CONFIGKEYFETCHSTORE_CONFIG;     // A secret value
    key_one.key = {
        .data = (uint8_t*)"prefs",
        .length = 5
    };
    
    uint32_t item_count = 1;
    MvConfigKeyToFetch keys[item_count];
    keys[0] = key_one;
    
    MvConfigKeyFetchParams request;
    request.num_items = item_count;
    request.keys_to_fetch = keys;
    
    enum MvStatus status = mvSendConfigFetchRequest(handles.channel, &request);
    if (status != MV_STATUS_OKAY) {
        server_error("Could not issue config fetch request");
        Channel::close();
        return false;
    }

    // Wait for the data to arrive
    server_log("Awaiting params...");
    received_config = false;
    uint32_t start_tick = HAL_GetTick();

    while (true) {
        // Break out after timeout or successful config retrieval
        if (received_config || (HAL_GetTick() - start_tick > CONFIG_WAIT_PERIOD_MS)) break;
    }
    
    if (!received_config) {
        // Request timed out
        server_error("Config fetch request timed out");
        Channel::close();
        return false;
    }

    // Parse the received data record
    server_log("Received params");
    MvConfigResponseData response;
    response.result = (MvConfigFetchResult)0;
    response.num_items = 0;
    
    status = mvReadConfigFetchResponseData(handles.channel, &response);
    if (status != MV_STATUS_OKAY || response.result != MV_CONFIGFETCHRESULT_OK || response.num_items != item_count) {
        server_error("Could not get config item (status: %i; result: %i)", status, response.result);
        Channel::close();
        return false;
    }
    
    uint8_t value[513] = {0};
    uint32_t value_length = 0;
    enum MvConfigKeyFetchResult result = (MvConfigKeyFetchResult)0;

    MvConfigResponseReadItemParams item;
    item.item_index = 0;
    item.result = &result;
    item.buf = {
        .data = &value[0],
        .size = 512,
        .length = &value_length
    };

    // Get the value itself
    status = mvReadConfigResponseItem(handles.channel, &item);
    if (status != MV_STATUS_OKAY || result != MV_CONFIGKEYFETCHRESULT_OK) {
        server_error("Could not get config item (status: %i; result: %i)", status, result);
        Channel::close();
        return false;
    }
    
    server_log("Received: %s", value);

    // Copy the value data to the requested location
    DynamicJsonDocument settings(512);
    DeserializationError err = deserializeJson(settings, value);
    if (err == DeserializationError::Ok) {
        prefs.mode          = (bool)settings["mode"];
        prefs.bst           = (bool)settings["bst"];
        prefs.colon         = (bool)settings["colon"];
        //prefs.flash         = (bool)settings["flash"];
        prefs.brightness    = (uint32_t)settings["brightness"];
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
    static volatile uint8_t config_rx_buffer[CONFIG_RX_BUFFER_SIZE_B] __attribute__((aligned(512)));
    static volatile uint8_t config_tx_buffer[CONFIG_TX_BUFFER_SIZE_B] __attribute__((aligned(512)));
    
    if (handles.channel == 0) {
        // No network connection yet? Then establish one
        Network::open();
        if (handles.network == 0) return false;
        
        // Get the network channel handle.
        // NOTE This is set in `logging.c` which puts the network in place
        //      (ie. so the network handle != 0) well in advance of this being called
        // Configure the required data channel
        MvOpenChannelParams channel_config;
        channel_config.version = 1;
        channel_config.v1 = {
            .notification_handle = handles.notification,
            .notification_tag    = USER_TAG_CONFIG_OPEN_CHANNEL,
            .network_handle      = handles.network,
            .receive_buffer      = (uint8_t*)config_rx_buffer,
            .receive_buffer_len  = sizeof(config_rx_buffer),
            .send_buffer         = (uint8_t*)config_tx_buffer,
            .send_buffer_len     = sizeof(config_tx_buffer),
            .channel_type        = MV_CHANNELTYPE_CONFIGFETCH,
            .endpoint            = {
                .data = (uint8_t*)"",
                .length = 0
            }
        };
        
        // Ask Microvisor to open the channel
        // and confirm that it has accepted the request
        enum MvStatus status = mvOpenChannel(&channel_config, &handles.channel);
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
        MvChannelHandle old_handle = handles.channel;
        enum MvStatus status = mvCloseChannel(&handles.channel);
        if (status == MV_STATUS_OKAY) {
            server_log("Config Channel closed (handle %lu)", old_handle);
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
    setup_notification_center();
    if (handles.notification == 0) return;
    
    // Check if we need to establish a network
    if (handles.network == 0) {
        // Configure the network connection request
        MvRequestNetworkParams network_config;
        network_config.version = 1;
        network_config.v1 = {
            .notification_handle = handles.notification,
            .notification_tag = USER_TAG_LOGGING_REQUEST_NETWORK,
        };
        
        // Ask Microvisor to establish the network connection
        // and confirm that it has accepted the request
        mvRequestNetwork(&network_config, &handles.network);

        // The network connection is established by Microvisor asynchronously,
        // so we wait for it to come up before opening the data channel -- which
        // would fail otherwise
        enum MvNetworkStatus net_status;
        while (1) {
            // Request the status of the network connection, identified by its handle.
            // If we're good to continue, break out of the loop...
            if (mvGetNetworkStatus(handles.network, &net_status) == MV_STATUS_OKAY && net_status == MV_NETWORKSTATUS_CONNECTED) {
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


/**
 * @brief Configure the network Notification Center.
 */
void setup_notification_center(void) {
    
    if (handles.notification == 0) {
        // Clear the notification store
        memset((void *)notification_center, 0xff, sizeof(notification_center));

        // Configure a notification center for network-centric notifications
        //MvNotificationSetup notification_config;
        //notification_config.irq = TIM8_BRK_IRQn,
        //notification_config.buffer = (MvNotification*)notification_center,
        //notification_config.buffer_size = sizeof(notification_center);

        static struct MvNotificationSetup notification_config = {
            .irq = TIM8_BRK_IRQn,
            .buffer = (struct MvNotification *)notification_center,
            .buffer_size = sizeof(notification_center)
        };

        // Ask Microvisor to establish the notification center
        // and confirm that it has accepted the request
        enum MvStatus status = mvSetupNotifications(&notification_config, &handles.notification);
        if (status == MV_STATUS_OKAY) {
            // Start the notification IRQ
            NVIC_ClearPendingIRQ(TIM8_BRK_IRQn);
            NVIC_EnableIRQ(TIM8_BRK_IRQn);
        } else {
            handles.notification = 0;
        }
    }
    
    server_log("Notification Center handle: %lu", handles.notification);
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
    bool got_notification = false;
    volatile MvNotification& notification = notification_center[notification_index];
    switch(notification.tag) {
        // Config fetch channel notifications
        case USER_TAG_CONFIG_OPEN_CHANNEL:
            if (notification.event_type == MV_EVENTTYPE_CHANNELDATAREADABLE) {
                // Flag we need to access received data and to close the channel
                // when we're back in the main loop. This lets us exit the ISR quickly.
                // Do NOT make Microvisor System Calls in the ISR!
                received_config = true;
                got_notification = true;
            }
            
            break;
        // Others
        default:
            break;
    }
    
    // We had a relevant notification
    if (got_notification) {
        // Point to the next record to be written
        notification_index = (notification_index + 1) % SHARED_NC_BUFFER_SIZE_R;

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
