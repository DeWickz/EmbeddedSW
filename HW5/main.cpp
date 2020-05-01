#include "mbed.h"
#include "TCPSocket.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "string"

Thread thread;
WiFiInterface *wifi;
TS_StateTypeDef  TS_State = {0};
Serial UART(SERIAL_TX, SERIAL_RX);

int arrivedcount = 0;
int data_count = 0;
int i=0;
char *data[5];


void get_payload(char payload[]){
    char* token = strtok(payload, " ");

    while(token)
    {
        data[data_count++] = token; 
        token = strtok(NULL, " ");
    }

    for(i = 0; i < 5; i++)
    {
        if(i == 0){printf("Confirmed = %s\n", data[0]);}
        else if(i == 1){printf("Recovered = %s\n", data[1]);}
        else if(i == 2){printf("Deaths = %s\n", data[2]);}
        else if(i == 3){printf("Hospitalized = %s\n", data[3]);}
        else if(i == 4){printf("Date = %s\n", data[4]);}
        printf("\n");
    }
    
    //DISPLAY_SETTING
    //CLEAR
    BSP_LCD_ClearStringLine(4);
    BSP_LCD_ClearStringLine(6);
    BSP_LCD_ClearStringLine(8);
    BSP_LCD_ClearStringLine(10);
    BSP_LCD_ClearStringLine(12);

    //SHOW
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);

    BSP_LCD_DisplayStringAtLine(3,(uint8_t*) "Confirmed: ");
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAtLine(4,(uint8_t*) data[0]);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);


    BSP_LCD_DisplayStringAtLine(5,(uint8_t*) "Recovered: ");
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAtLine(6,(uint8_t*) data[1]);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);


    BSP_LCD_DisplayStringAtLine(7,(uint8_t*) "Deaths: ");
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAtLine(8,(uint8_t*) data[2]);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    BSP_LCD_DisplayStringAtLine(9,(uint8_t*) "Hospitalized: ");
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAtLine(10,(uint8_t*) data[3]);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    BSP_LCD_DisplayStringAtLine(11,(uint8_t*) "Date: ");
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAtLine(12,(uint8_t*) data[4]);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
}

void subscribeCallback(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    // printf("Payload %.*s\n", message.payloadlen, (char*)message.payload);
    get_payload((char*)message.payload);
    arrivedcount++;
}

void display(){
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize()); 
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK); 
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"COVID 19 INFO", CENTER_MODE);
       
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);  
}

int main()
{
    UART.baud(115200);
    BSP_LCD_Init();
    thread.start(display);

    char* topic = "/Covid19";

    printf("WiFi MQTT example\n");

    #ifdef MBED_MAJOR_VERSION
        printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    #endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) 
    {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) 
    {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());

    MQTTNetwork mqttNetwork(wifi);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "192.168.1.35";
    int port = 1883;
    printf("[Sub] Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
    {
        printf("[Sub] rc from TCP connect is %d\r\n", rc);
    }
        
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-subscribe";
    data.username.cstring = "";
    data.password.cstring = "";


    if ((rc = client.connect(data)) != 0)
    {
        printf("[Sub]rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("[Sub]Client Connected.\r\n");
    }

    if ((rc = client.subscribe(topic, MQTT::QOS0, subscribeCallback)) != 0)
    {
        printf("[Sub]rc from MQTT subscribe is %d\r\n", rc);
    }
    else
    {
        printf("[Sub]Client subscribed.\r\n");
    }
        

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;

    int test_subscribe = 0;
    while(true){
        client.yield(10000);
        // printf("Test subscribe :: %d\n", test_subscribe);
        test_subscribe += 1; 
    }
    
    printf("[Sub]Finishing with %d messages received\n", arrivedcount);
    
    mqttNetwork.disconnect();

    if ((rc = client.disconnect()) != 0)
    {
        printf("[Sub]rc from disconnect was %d\r\n", rc);
        printf("[Sub]Client Disconnected.\r\n");
    }

    wifi->disconnect();

    printf("\n[Sub]Done\n");
}