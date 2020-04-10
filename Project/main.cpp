#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "TCPSocket.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "proDrawBoard.h"
#include "ResetReason.h"
#include <cstdlib>
#include <string>

WiFiInterface *wifi;
Thread thread;
Timer t;
int arrivedcount = 0;
const uint32_t TIMEOUT_MS = 30000; // Watchdog Timer

#define PressButton 1
DigitalIn  User_Button(PA_0,PullNone);

std::string reset_reason_to_string(const reset_reason_t reason)
{
    switch (reason) {
        case RESET_REASON_POWER_ON:
            return "Power On";
        case RESET_REASON_PIN_RESET:
            return "Hardware Pin";
        case RESET_REASON_SOFTWARE:
            return "Software Reset";
        case RESET_REASON_WATCHDOG:
            return "Watchdog";
        default:
            return "Other Reason";
    }
}

void subscribeCallback(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("\nFrom /Sub\n");
    //printf("Sub to Node-Red: Message received: qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    printf("Sub to Node-Red: Payload: %.*s\n\n", message.payloadlen, (char*)message.payload);

    drawSubMsg();
    wait(2);
    drawTopbar();
    ++arrivedcount;
}

void subscribe() {
    int count = 0;
    char* topic = "/Sub";

    wifi = WiFiInterface::get_default_instance();
    MQTTNetwork mqttNetwork(wifi);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "192.168.1.34";
    int port = 1883;
    printf("Sub to Node-Red: Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
    {
        printf("Sub to Node-Red: rc from TCP connect is %d\r\n", rc);
    }
        
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sub";
    data.username.cstring = "";
    data.password.cstring = "";
    
    if ((rc = client.connect(data)) != 0)
    {
        printf("Sub to Node-Red: rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("Sub to Node-Red: Client Connected.\r\n");
    }
        

    if ((rc = client.subscribe(topic, MQTT::QOS0, subscribeCallback)) != 0)
    {
        printf("Sub to Node-Red: rc from MQTT subscribe is %d\r\n", rc);
    }
    else
    {
        printf("Sub to Node-Red: Client subscribed.\r\n");
    }
        

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    while(1){
        client.yield(100000); // Wait for messages to be received
    }

    printf("Sub to Node-Red: Finishing with %d messages received\n", arrivedcount);
    printf("Sub to Node-Red:Done\n\n");
}

int main()
{

    int count = 0;
    char* topicSafe = "/TempHumid";
    char* lineNotify = "/LineNotify";
    char* lightTopic = "/Light";
    char* smokeTopic = "/Smoke";
    char* soundTopic = "/Sound";
    char* uploadTopic = "/Upload";
    char* runDuration = "/Duration";
    char buf[100];

    const reset_reason_t reason = ResetReason::get();
    printf("Last system reset reason: %s\r\n", reset_reason_to_string(reason).c_str());

    printf("Watchdog int.\n");
    Watchdog &watchdog = Watchdog::get_instance(); // Watchdog setup
    printf("Loop Start.\n\n");

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

    const char* hostname = "192.168.1.34";
    int port = 1883;
    printf("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
    {
        printf("Board pub: rc from TCP connect is %d\r\n", rc);
    }
        
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sample";
    data.username.cstring = "";
    data.password.cstring = "";
    
    if ((rc = client.connect(data)) != 0)
    {
        printf("Board pub: rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("Board pub: Client Connected.\r\n");
    }

    thread.start(subscribe);
        

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;

    srand(time(NULL));
    int timeSpent;
    DigitalOut led1(LED1);
    TS_StateTypeDef  TS_State = {0}; 

    uint16_t x1, y1;
    BSP_LCD_Init();

    /* Touchscreen initialization */
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }

    /* Clear the LCD */
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    /* Set Touchscreen Top title */
    BSP_LCD_SetTextColor(LCD_COLOR_DARKMAGENTA);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKMAGENTA);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"Haunted House Control", CENTER_MODE);

    //YELLOW    = LIGHT, TOP LEFT
    drawLightYellow();

    //LIGHTBLUE = SMOKE, TOP RIGHT
    drawSmokeLB();

    //GREEN     = SOUND FX, BOTTOM LEFT
    drawSFXGreen();

    //GRAY      = UPLOAD, BOTTOM RIGHT
    drawUploadGray(); 

    int lightState = 0;
    int smokeState = 0;
    int soundState = 0;
    int uploadState  = 0;
    int completed = 0;
    int left = 0;
    float duration = 0;
    
    string lightmsg, smokemsg, soundmsg, uploadmsg;

    while (true) {
        if(User_Button.read() == PressButton) // pressed button
        {
            printf("Press Switch to Restart now.");
            thread_sleep_for(50);
            watchdog.start(1); // Watchdog restart now
        }
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
            /* One or dual touch have been detected */
            /* Get X and Y position of the first touch post calibrated */
            x1 = TS_State.touchX[0];
            y1 = TS_State.touchY[0];
                        
            //Button state switching
            if((x1>=0)&&(x1<=120)&&(y1>=40)&&(y1<=160)) {//LIGHT 
                printf("Light switch");
                t.start();
                if(lightState==0){
                    lightState=1;
                    printf(" on\n");

                    lightmsg = "On";
                    sprintf(buf, lightmsg.c_str(), count);
                    message.payload = buf;
                    message.payloadlen = strlen(buf);
                    rc = client.publish(lightTopic, message);
                    drawLightWhite();
                    wait(0.1);

                }
                else if(lightState==1){
                    lightState=0;
                    printf(" off\n");

                    lightmsg = "Off";
                    sprintf(buf, lightmsg.c_str(), count);
                    message.payload = buf;
                    message.payloadlen = strlen(buf);
                    rc = client.publish(lightTopic, message);
                    drawLightYellow();
                    wait(0.1);
                    left++;
                    t.stop();
                    t.reset();
                }
                wait(0.5);
            }
            else if ((x1>=121)&&(x1<=240)&&(y1>=40)&&(y1<=160)) {//SMOKE
                printf("Smoke switch");
                if(smokeState==0){
                    smokeState=1;
                    printf(" on\n");

                    smokemsg = "On";
                    sprintf(buf, smokemsg.c_str(), count);
                    message.payload = buf;
                    message.payloadlen = strlen(buf);
                    rc = client.publish(smokeTopic, message);
                    drawSmokeWhite();
                    wait(0.1);
                }
                else if(smokeState==1){
                    smokeState=0;
                    printf(" off\n");

                    smokemsg = "Off";
                    sprintf(buf, smokemsg.c_str(), count);
                    message.payload = buf;
                    message.payloadlen = strlen(buf);
                    rc = client.publish(smokeTopic, message);
                    drawSmokeLB();

                    lightState=0;
                    lightmsg = "Off";
                    sprintf(buf, lightmsg.c_str(), count);
                    message.payload = buf;
                    message.payloadlen = strlen(buf);
                    rc = client.publish(lightTopic, message);
                    drawLightYellow();
                    wait(0.1);
                    left++; 
                    t.stop();
                    t.reset();
                }
                wait(0.5);
            }
            else if ((x1>=0)&&(x1<=120)&&(y1>=161)&&(y1<=240)){//SFX
                printf("Sound switch");
                if(soundState==0){
                    soundState=1;
                    printf(" on\n");

                    soundmsg = "On";
                    sprintf(buf, soundmsg.c_str(), count);
                    message.payload = buf;
                    message.payloadlen = strlen(buf);
                    rc = client.publish(soundTopic, message);
                    drawSFXWhite();
                    wait(0.1);  
                }
                else if(soundState==1){
                    if(lightState==1 && smokeState==1)
                    {
                        soundState=0;
                        printf(" off\n");
                        soundmsg = "Off";
                        sprintf(buf, soundmsg.c_str(), count);
                        message.payload = buf;
                        message.payloadlen = strlen(buf);
                        rc = client.publish(soundTopic, message);
                        drawSFXGreen();

                        smokeState=0;
                        smokemsg = "Off";
                        sprintf(buf, smokemsg.c_str(), count);
                        message.payload = buf;
                        message.payloadlen = strlen(buf);
                        rc = client.publish(smokeTopic, message);
                        drawSmokeLB();
                        
                        lightState=0;
                        lightmsg = "Off";
                        sprintf(buf, lightmsg.c_str(), count);
                        message.payload = buf;
                        message.payloadlen = strlen(buf);
                        rc = client.publish(lightTopic, message);
                        drawLightYellow();
                        wait(0.1);
                        
                        t.stop();
                        printf("\nRun duration: %f\n", t.read());
                        duration = t.read();

                        string runTemp = to_string(duration);
                        sprintf(buf, runTemp.c_str(), count);
                        message.payload = buf;
                        message.payloadlen = strlen(buf);
                        rc = client.publish(runDuration, message);
                        t.reset();
                        completed++;
                    }
                    else {
                        soundState=0;
                        printf(" off\n");
                        soundmsg = "Off";
                        sprintf(buf, soundmsg.c_str(), count);
                        message.payload = buf;
                        message.payloadlen = strlen(buf);
                        rc = client.publish(soundTopic, message);
                        drawSFXGreen();

                        t.stop();
                        t.reset();
                    }
                    
                }
                wait(0.5);
            }
            else if ((x1>=121)&&(x1<=240)&&(y1>=161)&&(y1<=240)){//UPLOAD
                printf("\nUploading\n");
                
                string part1 = "{\"Completed\":\"";
                string part2 = "\",\"Left\":\"";
                string part3 = "\"}";
                string uploadmsg = part1 + to_string(completed) + part2 + to_string(left) + part3;
                sprintf(buf, uploadmsg.c_str(), count);
                message.payload = buf;
                message.payloadlen = strlen(buf);
                rc = client.publish(lineNotify, message);
                rc = client.publish(uploadTopic, message);

                drawUploadWhite();
                wait(1);  
                led1.write(1);
                printf("\nUploaded\n");
                drawUploadGrayUploaded();
                wait(1);
                led1.write(0);
                drawUploadGray();
                wait(0.1);
                drawSFXGreen();
                drawLightYellow();
                drawSmokeLB();

                lightState = 0;
                smokeState = 0;
                soundState = 0;
                uploadState  = 0;
                completed = 0;
                left = 0;
                wait(0.5);
            }

        }
        else {
            
        }
    }
} 