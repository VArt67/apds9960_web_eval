
#include "web.h"
#include <dbg.h>

#ifdef ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#else
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#endif

ESP8266WebServer server(80);

// SSID and Password of your WiFi router
const char *ssid = "FamArt";
const char *password = "57551317930074617623";

#define _MAX_PAGE_BUF_SIZE  (32*1024)

void handleRoot2(void)
{
  
}

Web::Web()
{
    //htmlPage=MAIN_page;  // default index.html
    htmlPageBuf = NULL;
    htmlPageBufLength = 0;

    getApiSetupData = NULL;
    getApiOutputData = NULL;
    setApiSetupData = NULL;
    setApiLLSetupData = NULL;
}

void Web::setup()
{
    WiFi.hostname("APDS9960Test");
    WiFi.begin(ssid, password); // Connect to your WiFi router

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        ESP_LOGI_N(TAG,".");
    }
    ESP_LOGI(TAG, "\n");
    ESP_LOGI(TAG,"Connected to\t%s",WiFi.SSID().c_str()); // Tell us what network we're connected to
    ESP_LOGI(TAG,"IP address:\t%s",WiFi.localIP().toString().c_str()); // Send the IP address of the ESP8266 to the computer

    server.on("/", HTTP_GET, std::bind(&Web::handleRoot, this));
    server.on("/api/setup", HTTP_GET, std::bind(&Web::handleGetSetupData, this));
    server.on("/api/pollSensorOutput", HTTP_GET, std::bind(&Web::handleOutput, this));
    server.on("/save", HTTP_POST, std::bind(&Web::handleSetSetupData, this));
    server.on("/save", HTTP_PUT, std::bind(&Web::handleSetSetupData, this));
    server.on("/save", HTTP_GET, std::bind(&Web::handleSetSetupData, this));
    server.on("/saveLowLevel", HTTP_POST, std::bind(&Web::handleSetLLSetupData, this));
    server.on("/saveLowLevel", HTTP_PUT, std::bind(&Web::handleSetLLSetupData, this));
    server.on("/saveLowLevel", HTTP_GET, std::bind(&Web::handleSetLLSetupData, this));

    // handle new html page upload
    // this page will be the new page to display (until rebbot)
    server.on("/upload", HTTP_GET, std::bind(&Web::handleFileRead, this));  
    // if the client posts to the upload page
    //server.on("/upload", HTTP_POST, std::bind(&Web::handleFileUpload, this)); // Receive and save the file
    server.on(
        "/upload", HTTP_POST, // if the client posts to the upload page
        []()
        { server.send(200); },                  // Send status 200 (OK) to tell the client we are ready to receive
        std::bind(&Web::handleFileUpload, this) // Receive and save the file
    );

    server.onNotFound(std::bind(&Web::handleNotFound, this));

    server.begin(); // Start server
}


void Web::loop()
{
    server.handleClient(); // Handle client requests
}

////########################################################
////########################################################

// Manage not found URL
void Web::handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

////########################################################
#include "index.html"
void Web::handleRoot(void)
{
    if (htmlPageBuf && htmlPageBufLength > 10)
    {
        ESP_LOGD(TAG,"handleRoot send uploaded page");
        server.send(200, "text/html", htmlPageBuf);
    }
    else
    {
        ESP_LOGD(TAG,"handleRoot send default page");
        String s = MAIN_page;
        server.send(200, "text/html", s);
    }
}

////########################################################
void Web::handleGetSetupData(void)
{
    ESP_LOGD(TAG,"handleApi");
    DynamicJsonDocument doc(2048);

    if (getApiSetupData)
        (*getApiSetupData)(doc);

    String buf;
    serializeJson(doc, buf);
    server.send(200, F("application/json"), buf);

    ESP_LOGD(TAG,buf.c_str());
}

void Web::handleOutput(void)
{
    ESP_LOGD(TAG,"handleOutput");
    DynamicJsonDocument doc(2048);

    if (getApiOutputData)
        (*getApiOutputData)(doc);

    String buf;
    serializeJson(doc, buf);
    ESP_LOGV_N(TAG, "o");
    server.send(200, F("application/json"), buf);
}

void Web::handleSetSetupData(void)
{
    ESP_LOGD(TAG,"handleApiUpdate");

    if (setApiSetupData)
    {
        DynamicJsonDocument doc(2048);

        int numArgs = server.args();
        ESP_LOGD(TAG, "NumArgs: %ds", numArgs);

        for (int i = 0; i < numArgs; i++)
        {
            String an = server.argName(i);
            String a = server.arg(i);
            doc[an] = a;

            ESP_LOGV(TAG, "Arg %d: %s=%s", i, an.c_str(), a.c_str());
        }
        (*setApiSetupData)(doc);
    }
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
}

void Web::handleSetLLSetupData(void)
{
    ESP_LOGD(TAG,"handleApiLLUpdate");

    if (setApiLLSetupData)
    {
        DynamicJsonDocument doc(2048);

        int numArgs = server.args();
        ESP_LOGD(TAG, "NumArgs: %d", numArgs);

        for (int i = 0; i < numArgs; i++)
        {
            String an = server.argName(i);
            String a = server.arg(i);
            doc[an] = a;

            ESP_LOGV(TAG, "Arg %d: %s=%s", i, an.c_str(), a.c_str());
        }
        (*setApiLLSetupData)(doc);
    }
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
}

// ################################################################
// for debugging the htmnl we can upload a new page to be displayed

void Web::handleFileRead()
{
        // send the right file to the client (if it exists)
        ESP_LOGD(TAG,"handleFileRead");
#if 0
        handleRoot(); // load last index.html
#else
        String s = F("<p><a href=\"/\">Redirect</a></p>");
        server.send(200, "text/html", s);
#endif
}

void Web::handleFileUpload()
{
    // Send status 200 (OK) to tell the client we are ready to receive
    //server.send(200);
    HTTPUpload &upload = server.upload();

    ESP_LOGD(TAG, "handleFileUpload status: %d size: %d", upload.status, upload.currentSize);

    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        ESP_LOGD(TAG, "handleFileUpload Name: %s", filename.c_str());
        if (NULL==htmlPageBuf)
            htmlPageBuf=(char*)malloc(_MAX_PAGE_BUF_SIZE);
        // Clear string content
        htmlPageBufLength = 0;
        htmlPageBuf[htmlPageBufLength] = 0;
        //htmlPage.clear(); 
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
        {
           if ((htmlPageBufLength + upload.currentSize) < (_MAX_PAGE_BUF_SIZE-1))
           {
              char *cp = htmlPageBuf + htmlPageBufLength;
              memcpy(cp, upload.buf, upload.currentSize);
              htmlPageBufLength = htmlPageBufLength + upload.currentSize;
              htmlPageBuf[htmlPageBufLength] = 0;
           }
           else
           {
              ESP_LOGE(TAG, "handleFileUpload: size overflow");
              htmlPageBuf[0] = 0; // purge all
              htmlPageBufLength = 0;
           }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
           ESP_LOGD(TAG, "handleFileUpload Total: %d", upload.totalSize);
           // htmlPage = htmlPageBuf;
           // server.sendHeader("Location", "/success.html"); // Redirect the client to the success page
           server.sendHeader("Location", "/"); // Reload index.html
           server.send(303);
        }
}