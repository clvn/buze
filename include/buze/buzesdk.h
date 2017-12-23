#pragma once

class CApplication;
class CViewFrame;
class CViewInfo;
class CView;
class CEventHandler;
class CHostModule;
class CDocument;
class CConfiguration;

#define NO_BUZE_APPLICATION_TYPE
typedef CApplication buze_application_t;

#define NO_BUZE_MAIN_FRAME_TYPE
typedef CViewFrame buze_main_frame_t;

#define NO_BUZE_WINDOW_FACTORY_TYPE
typedef CViewInfo buze_window_factory_t;

#define NO_BUZE_WINDOW_TYPE
typedef CView buze_window_t;

#define NO_BUZE_EVENT_HANDLER_TYPE
typedef CEventHandler buze_event_handler_t;

#define NO_BUZE_DOCUMENT_TYPE
typedef CDocument buze_document_t;

#define NO_BUZE_CONFIGURATION_TYPE
typedef CConfiguration buze_configuration_t;

#define NO_BUZE_HOST_MODULE_TYPE
typedef CHostModule buze_host_module_t;

#include <zzub/zzub.h>
#include "buze.h"
#include "View.h"
