/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

        LOWI Event Receiver

GENERAL DESCRIPTION
  This file contains the implementation of LOWI Event Receiver

Copyright (c) 2012-2013, 2016,2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

(c) 2012-2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <base_util/log.h>
#include <lowi_server/lowi_event_receiver.h>
#include <base_util/vector.h>
#include <base_util/log.h>
#include <base_util/postcard.h>
#include <mq_client/mq_client.h>
#include <common/lowi_utils.h>
#include "lowi_measurement_result.h"
#include <lowi_server/lowi_internal_message.h>

using namespace qc_loc_fw;

const char * const LOWIEventReceiver::TAG = "LOWIEventReceiver";

LOWIEventReceiver::LOWIEventReceiver()
: mListener (NULL)
{
}

LOWIEventReceiver::~LOWIEventReceiver()
{
}

void LOWIEventReceiver::addListener (LOWIEventReceiverListener* pListener)
{
  mListener = pListener;
}

void LOWIEventReceiver::removeListener (LOWIEventReceiverListener* /* pListener */)
{
  mListener = NULL;
}

LOWILocalMsg*
LOWIEventReceiver::handleEvent (InPostcard * const card)
{
  LOWILocalMsg* msg = NULL;
  LOWIRequest* request = NULL;
  bool success = false;
  do
  {
    if (NULL == card) break;

    const char* print_from = NOT_AVAILABLE;
    const char * from = 0;
    if (0 == card->getString("FROM", &from))
    {
      print_from = from;
    }

    const char* print_to = NOT_AVAILABLE;
    const char * to = 0;
    if (0 == card->getString("TO", &to))
    {
      print_to = to;
    }

    const char* print_req = NOT_AVAILABLE;
    const char * req = 0;
    if (0 == card->getString("REQ", &req))
    {
      print_req = req;
    }

    unsigned int req_id = 0;
    if (0 != card->getUInt32("REQ_ID", req_id))
    {
      log_verbose (TAG, "Unable to extract REQ_ID");
    }

    const char* print_resp = NOT_AVAILABLE;
    const char * resp = 0;
    if (0 == card->getString("RESP", &resp))
    {
      print_resp = resp;
    }

    const char* print_info = NOT_AVAILABLE;
    const char * info = 0;
    if (0 == card->getString("INFO", &info))
    {
      print_info = info;
    }

    const char *print_ireq = NOT_AVAILABLE;
    const char *ireq       = 0;
    if (0 == card->getString("IMSG", &ireq))
    {
      print_ireq = ireq;
    }

    log_debug(TAG, "%s:FROM:%s, TO:%s, REQ:%s, IREQ:%s, REQ_ID:%d, RESP:%s,"
             " INFO:%s", __FUNCTION__,
             print_from, print_to, print_req, print_ireq, req_id, print_resp, print_info);

    // Check if it is a request, a scan measurement or an internal message
    if (0 != req)
    {
      // A request
      request = LOWIUtils::inPostcardToRequest (card);
      if (NULL == request)
      {
        log_error (TAG, "%s, Unable to create request!", __func__);
        break;
      }

      msg = new (std::nothrow) LOWILocalMsg (request);
      if (NULL == msg)
      {
        log_error (TAG, "%s, Unable to create LOWILocalMsg!", __func__);
        delete request;
        break;
      }
      success = true;
      const char *iface = 0;
      if((0 == card->getString("INTERFACE", &iface))) {
          request->set_interface(iface);
      }
    }
    else if (0 != resp)
    {
      log_debug(TAG, "Response to [%s]", resp);
      if (NULL == mListener)
      {
        log_warning (TAG, "No listener registered to receive wifi events");
        break;
      }
      else
      {
        // Unknown response that we are not interested in.
        break;
      }
      success = true;
    }
    else if (0 != info)
    {
      log_debug(TAG, "Info to [%s]", info);
      if (NULL == mListener)
      {
        log_warning (TAG, "No listener registered to receive wifi events");
        break;
      }

      if (0 == strcmp("INTF-STATUS-UPDATE", info))
      {
        LOWIDriverInterface intf;
        const char *tmp = 0;
        int32 ifstate = (int32)INTF_UNKNOWN;
        if ((0 == card->getString("IFNAME", &tmp)) &&
            (0 == card->getInt32("IS_INTF_ON", ifstate)))
        {
          strlcpy(intf.ifname, tmp, LOWI_MAX_INTF_NAME_LEN);
          intf.state = (eWifiIntfState)ifstate;
          log_verbose(TAG, "%s: ifname(%s) ifstate(%d)", __FUNCTION__, intf.ifname, intf.state);
          mListener->updateIntfState(intf);
        }
        else
        {
          log_verbose(TAG, "%s: could not retrieve intf name and/or state", __FUNCTION__);
        }
      }
      success = true;
    }
    else if (0 != ireq)
    {
      // Parse internal message
      LOWIInternalMessage *r = LOWIInternalMessage::parseInternalMessage(card);
      if (NULL == r)
      {
        log_error (TAG, "%s, Unable to create internal message!", __func__);
        break;
      }

      msg = new (std::nothrow) LOWILocalMsg (r);
      if (NULL == msg)
      {
        log_error (TAG, "%s, Unable to create LOWILocalMsg!", __func__);
        delete r;
        break;
      }
      success = true;
    }
    else
    {
      // Should be a Scan Measurement if not a request
      LOWIMeasurementResult* result =
          LOWIMeasurementResultBase::parseScanMeasurement (card);
      if (NULL == result)
      {
        log_error (TAG, "%s, Unable to create measurement result!", __func__);
        break;
      }

      msg = new (std::nothrow) LOWILocalMsg (result);
      if (NULL == msg)
      {
        log_error (TAG, "%s, Unable to create LOWILocalMsg!", __func__);
        delete result;
        break;
      }
      success = true;
    }
  } while(0);

  if(false == success)
  {
    log_error(TAG, "handleEvent failed");
  }
  return msg;
}
