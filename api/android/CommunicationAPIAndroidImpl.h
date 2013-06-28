
#pragma once

#include "../CommunicationAPI.h"
#include "JNIWrapper.h"

namespace jni_comm_api {
    enum Enum {
        GiftizMissionDone,
        GiftizGetButtonState,
        GiftizButtonClicked,
        ShareFacebook,
        ShareTwitter,
        MustShowRateDialog,
        RateItNow,
        RateItLater,
        RateItNever,
    };
}
class CommunicationAPIAndroidImpl : public CommunicationAPI, public JNIWrapper<jni_comm_api::Enum> {
    public:
        CommunicationAPIAndroidImpl();

        void init(JNIEnv* env);

        void giftizMissionDone();
        int  giftizGetButtonState();
        void giftizButtonClicked();

        void shareFacebook();
        void shareTwitter();

        bool mustShowRateDialog();
        void rateItNow();
        void rateItLater();
        void rateItNever();
    public:
        JNIEnv* env;
};
