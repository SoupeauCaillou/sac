#include "MusicAPIAndroidImpl.h"
#include "../../base/Log.h"
#include "../../base/MathUtil.h"
#include "../../systems/MusicSystem.h"

struct AndroidOpaquePtr : public OpaqueMusicPtr {
	jobject audioTrack;
	int queuedSize;
};
struct MusicAPIAndroidImpl::MusicAPIAndroidImplData {	
	jclass javaMusicApi;
	jmethodID createPlayer;
    jmethodID queueMusicData;
    jmethodID startPlaying;
    jmethodID stopPlayer;
    jmethodID getPosition;
    jmethodID setPosition;
    jmethodID setVolume;
    jmethodID deletePlayer;
    jmethodID isPlaying;
};

MusicAPIAndroidImpl::MusicAPIAndroidImpl(JNIEnv *pEnv) : env(pEnv) {
	
}

void MusicAPIAndroidImpl::init() {
	datas = new MusicAPIAndroidImplData();

	datas->javaMusicApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib"));
	datas->createPlayer = env->GetStaticMethodID(datas->javaMusicApi, "createPlayer", "(I)Ljava/lang/Object;");
	datas->queueMusicData = env->GetStaticMethodID(datas->javaMusicApi, "queueMusicData", "(Ljava/lang/Object;[BII)V");
	datas->startPlaying = env->GetStaticMethodID(datas->javaMusicApi, "startPlaying", "(Ljava/lang/Object;Ljava/lang/Object;I)V");
	datas->stopPlayer = env->GetStaticMethodID(datas->javaMusicApi, "stopPlayer", "(Ljava/lang/Object;)V");
	datas->getPosition = env->GetStaticMethodID(datas->javaMusicApi, "getPosition", "(Ljava/lang/Object;)I");
	datas->setPosition = env->GetStaticMethodID(datas->javaMusicApi, "setPosition", "(Ljava/lang/Object;I)V");
	datas->setVolume = env->GetStaticMethodID(datas->javaMusicApi, "setVolume", "(Ljava/lang/Object;F)V");
	datas->isPlaying = env->GetStaticMethodID(datas->javaMusicApi, "isPlaying", "(Ljava/lang/Object;)Z");
	datas->deletePlayer = env->GetStaticMethodID(datas->javaMusicApi, "deletePlayer", "(Ljava/lang/Object;)V");
}

OpaqueMusicPtr* MusicAPIAndroidImpl::createPlayer(int sampleRate) {
	AndroidOpaquePtr* ptr = new AndroidOpaquePtr();
	ptr->audioTrack = env->NewGlobalRef(env->CallStaticObjectMethod(datas->javaMusicApi, datas->createPlayer, sampleRate));
	ptr->queuedSize = 0;
	return ptr;
}

void MusicAPIAndroidImpl::queueMusicData(OpaqueMusicPtr* _ptr, int8_t* data, int size, int sampleRate) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	
	jbyteArray jdata;
	jdata = env->NewByteArray(size);
	env->SetByteArrayRegion(jdata, 0,  size, (jbyte *)data);
	env->CallStaticVoidMethod(datas->javaMusicApi, datas->queueMusicData, ptr->audioTrack, jdata, size, sampleRate);
	env->DeleteLocalRef(jdata);
	ptr->queuedSize += size;
}

bool MusicAPIAndroidImpl::needData(OpaqueMusicPtr* _ptr, int sampleRate) {
	if (!isPlaying(_ptr))
		return false;

    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    int queuedSamples = ptr->queuedSize / 2;
    int pos = getPosition(ptr);
    //LOGI("%p) NEED DATA: (%d + %d) >= %d", ptr, pos, SEC_TO_SAMPLES(0.5, sampleRate), queuedSamples);
    return (pos + SEC_TO_SAMPLES(0.5, sampleRate) >= queuedSamples);
}

void MusicAPIAndroidImpl::startPlaying(OpaqueMusicPtr* _ptr, OpaqueMusicPtr* master, int offset) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	env->CallStaticVoidMethod(
		datas->javaMusicApi, 
		datas->startPlaying, 
		ptr->audioTrack, 
		master ? (static_cast<AndroidOpaquePtr*>(master))->audioTrack : 0, 
		offset);
}

void MusicAPIAndroidImpl::stopPlayer(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	env->CallStaticVoidMethod(datas->javaMusicApi, datas->stopPlayer, ptr->audioTrack);
}

int MusicAPIAndroidImpl::getPosition(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	return env->CallStaticIntMethod(datas->javaMusicApi, datas->getPosition, ptr->audioTrack);
}

void MusicAPIAndroidImpl::setPosition(OpaqueMusicPtr* _ptr, int pos) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	LOGI("setPosition(%d) : queued:%d", pos, ptr->queuedSize);
	env->CallStaticVoidMethod(datas->javaMusicApi, datas->setPosition, ptr->audioTrack, pos);
}

void MusicAPIAndroidImpl::setVolume(OpaqueMusicPtr* _ptr, float v) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	env->CallStaticVoidMethod(datas->javaMusicApi, datas->setVolume, ptr->audioTrack, v);
}

void MusicAPIAndroidImpl::deletePlayer(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	env->CallStaticVoidMethod(datas->javaMusicApi, datas->deletePlayer, ptr->audioTrack);
	env->DeleteGlobalRef(ptr->audioTrack);
	delete ptr;
}

bool MusicAPIAndroidImpl::isPlaying(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	int p = getPosition(_ptr);
	// LOGI("%p) %d / %d = ratio: %.3f", ptr, p, ptr->queuedSize, (2*p / (float)ptr->queuedSize));
	if (p && ((2*p / (float)ptr->queuedSize) >= 0.999))
		return false;
	return env->CallStaticBooleanMethod(datas->javaMusicApi, datas->isPlaying, ptr->audioTrack);
}
