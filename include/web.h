
#ifndef __WEB_H__
#define __WEB_H__
#include <ArduinoJson.h>

class Web
{
    public:
        Web(void);
        void setup();
        void loop();

        void (*getApiSetupData)(DynamicJsonDocument &doc);
        void (*setApiSetupData)(DynamicJsonDocument &doc);
        void (*setApiLLSetupData)(DynamicJsonDocument &doc);

        void (*getApiOutputData)(DynamicJsonDocument &doc);

    private:
        void handleRoot(void);
        void handleNotFound();

        void handleGetSetupData(void);
        void handleOutput(void);
        void handleSetSetupData(void);
        void handleSetLLSetupData(void);

        void handleFileRead();
        void handleFileUpload();
        //std::string htmlPage;
        char *htmlPageBuf;
        uint16_t htmlPageBufLength;
};


#endif