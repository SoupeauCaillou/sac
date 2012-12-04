/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "MusicAPIAndroidImpl.h"
#include "../../base/Log.h"
#include "../../base/MathUtil.h"
#include "../../systems/MusicSystem.h"
#include <map>

struct AndroidOpaquePtr : public OpaqueMusicPtr {
	jobject audioTrack;
	int queuedSize;
};
struct MusicAPIAndroidImpl::MusicAPIAndroidImplData {	
	jclass javaMusicApi;
	jmethodID createPlayer;
    jmethodID pcmBufferSize;
    jmethodID allocate;
    jmethodID deallocate;
    jmethodID queueMusicData;
    jmethodID startPlaying;
    jmethodID stopPlayer;
    jmethodID pausePlayer;
    jmethodID getPosition;
    jmethodID setPosition;
    jmethodID setVolume;
    jmethodID deletePlayer;
    jmethodID isPlaying;
    jmethodID initialPacketCount;


    std::map<int8_t*, jbyteArray> ptr2array;
    bool initialized;
};

MusicAPIAndroidImpl::MusicAPIAndroidImpl() {
	datas = new MusicAPIAndroidImplData();
	datas->initialized = false;
}

MusicAPIAndroidImpl::~MusicAPIAndroidImpl() {
    uninit();
    delete datas;
}

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}

void MusicAPIAndroidImpl::uninit() {
	if (datas->initialized) {
		env->DeleteGlobalRef(datas->javaMusicApi);
		datas->initialized = false;
	}
}

void MusicAPIAndroidImpl::init(JNIEnv* pEnv) {
	env = pEnv;

	datas->javaMusicApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/MusicAPI"));
	datas->createPlayer = jniMethodLookup(env, datas->javaMusicApi, "createPlayer", "(I)Ljava/lang/Object;");
    datas->pcmBufferSize = jniMethodLookup(env, datas->javaMusicApi, "pcmBufferSize", "(I)I");
    datas->allocate = jniMethodLookup(env, datas->javaMusicApi, "allocate", "(I)[B");
    datas->deallocate = jniMethodLookup(env, datas->javaMusicApi, "deallocate", "([B)V");
    datas->queueMusicData = jniMethodLookup(env, datas->javaMusicApi, "queueMusicData", "(Ljava/lang/Object;[BII)[B");
	datas->startPlaying = jniMethodLookup(env, datas->javaMusicApi, "startPlaying", "(Ljava/lang/Object;Ljava/lang/Object;I)V");
	datas->stopPlayer = jniMethodLookup(env, datas->javaMusicApi, "stopPlayer", "(Ljava/lang/Object;)V");
    datas->pausePlayer = jniMethodLookup(env, datas->javaMusicApi, "pausePlayer", "(Ljava/lang/Object;)V");
	datas->getPosition = jniMethodLookup(env, datas->javaMusicApi, "getPosition", "(Ljava/lang/Object;)I");
	datas->setPosition = jniMethodLookup(env, datas->javaMusicApi, "setPosition", "(Ljava/lang/Object;I)V");
	datas->setVolume = jniMethodLookup(env, datas->javaMusicApi, "setVolume", "(Ljava/lang/Object;F)V");
	datas->isPlaying = jniMethodLookup(env, datas->javaMusicApi, "isPlaying", "(Ljava/lang/Object;)Z");
	datas->deletePlayer = jniMethodLookup(env, datas->javaMusicApi, "deletePlayer", "(Ljava/lang/Object;)V");
    datas->initialPacketCount = jniMethodLookup(env, datas->javaMusicApi, "initialPacketCount", "(Ljava/lang/Object;)I");
    datas->initialized = true;
}

OpaqueMusicPtr* MusicAPIAndroidImpl::createPlayer(int sampleRate) {
	AndroidOpaquePtr* ptr = new AndroidOpaquePtr();
    jobject p = env->CallStaticObjectMethod(datas->javaMusicApi, datas->createPlayer, sampleRate);
    if (p) {
	    ptr->audioTrack = env->NewGlobalRef(p);
    } else {
        ptr->audioTrack = 0;
    }
	ptr->queuedSize = 0;

	return ptr;
} 

int MusicAPIAndroidImpl::pcmBufferSize(int sampleRate) {
    return env->CallStaticIntMethod(datas->javaMusicApi, datas->pcmBufferSize, sampleRate);
}

int8_t* MusicAPIAndroidImpl::allocate(int size) {
	return new int8_t[size];
	
    // retrieve byte[] from Java
    jbyteArray b = (jbyteArray) env->CallStaticObjectMethod(datas->javaMusicApi, datas->allocate, size);
    // buffer is either a copy or a direct pointer to underlying byte[] storage
    jbyte* buffer = env->GetByteArrayElements(b, 0);
    datas->ptr2array[buffer] = b;
    return buffer;
}

int MusicAPIAndroidImpl::initialPacketCount(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    return env->CallStaticIntMethod(datas->javaMusicApi, datas->initialPacketCount, ptr->audioTrack);
}

void MusicAPIAndroidImpl::deallocate(int8_t* b) {
    env->CallStaticVoidMethod(datas->javaMusicApi, datas->deallocate, datas->ptr2array[b]);
}

void MusicAPIAndroidImpl::queueMusicData(OpaqueMusicPtr* _ptr, int8_t* data, int size, int sampleRate) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);

    if (!ptr->audioTrack)
        return;

#if 1
	// retrieve byte[] from Java
    jbyteArray b = (jbyteArray) env->CallStaticObjectMethod(datas->javaMusicApi, datas->allocate, size);
    env->SetByteArrayRegion(b, 0, size, (jbyte*)data);
    delete[] data;
    
    env->CallStaticObjectMethod(datas->javaMusicApi, datas->queueMusicData, ptr->audioTrack, b, size, sampleRate);
#else
    jbyteArray jdata;

    std::map<int8_t*, jbyteArray>::iterator it = datas->ptr2array.find(data);
    if (it == datas->ptr2array.end()) {
        LOGW("THIS IS AN ERROR /o\\");
    } else {
        // jni case : commit change to byte[] buffer
        jdata = it->second;
        env->ReleaseByteArrayElements(jdata, data, 0);
    }

	jbyteArray b = (jbyteArray) env->CallStaticObjectMethod(datas->javaMusicApi, datas->queueMusicData, ptr->audioTrack, jdata, size, sampleRate);

    if (it == datas->ptr2array.end()) {
        env->DeleteLocalRef(jdata);
    } else {
	    datas->ptr2array.erase(it);
    }
	ptr->queuedSize += size;
#endif
}

void MusicAPIAndroidImpl::startPlaying(OpaqueMusicPtr* _ptr, OpaqueMusicPtr* master, int offset) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

	env->CallStaticVoidMethod(
		datas->javaMusicApi, 
		datas->startPlaying, 
		ptr->audioTrack, 
		master ? (static_cast<AndroidOpaquePtr*>(master))->audioTrack : 0, 
		offset);
     ptr->queuedSize = 0;
}

void MusicAPIAndroidImpl::stopPlayer(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

	env->CallStaticVoidMethod(datas->javaMusicApi, datas->stopPlayer, ptr->audioTrack);
}

void MusicAPIAndroidImpl::pausePlayer(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

    env->CallStaticVoidMethod(datas->javaMusicApi, datas->pausePlayer, ptr->audioTrack);
}

int MusicAPIAndroidImpl::getPosition(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return 0;

	return env->CallStaticIntMethod(datas->javaMusicApi, datas->getPosition, ptr->audioTrack);
}

void MusicAPIAndroidImpl::setPosition(OpaqueMusicPtr* _ptr, int pos) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;
	env->CallStaticVoidMethod(datas->javaMusicApi, datas->setPosition, ptr->audioTrack, pos);
}

void MusicAPIAndroidImpl::setVolume(OpaqueMusicPtr* _ptr, float v) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

	env->CallStaticVoidMethod(datas->javaMusicApi, datas->setVolume, ptr->audioTrack, v);
}

void MusicAPIAndroidImpl::deletePlayer(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
	env->CallStaticVoidMethod(datas->javaMusicApi, datas->deletePlayer, ptr->audioTrack);
    if (ptr->audioTrack)
        env->DeleteGlobalRef(ptr->audioTrack);
	delete ptr;
}

bool MusicAPIAndroidImpl::isPlaying(OpaqueMusicPtr* _ptr) {
	AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return false;
	return env->CallStaticBooleanMethod(datas->javaMusicApi, datas->isPlaying, ptr->audioTrack);
}
