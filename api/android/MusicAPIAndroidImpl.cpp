/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "MusicAPIAndroidImpl.h"
#include "../../systems/MusicSystem.h"
#include <map>

struct AndroidOpaquePtr : public OpaqueMusicPtr {
    jobject audioTrack;
    int queuedSize;
};

MusicAPIAndroidImpl::MusicAPIAndroidImpl() : JNIWrapper<jni_music_api::Enum>("net/damsy/soupeaucaillou/api/MusicAPI", true) {
    declareMethod(jni_music_api::CreatePlayer, "createPlayer", "(I)Ljava/lang/Object;");
    declareMethod(jni_music_api::PcmBufferSize, "pcmBufferSize", "(I)I");
    declareMethod(jni_music_api::Allocate, "allocate", "(I)[S");
    declareMethod(jni_music_api::Deallocate, "deallocate", "([S)V");
    declareMethod(jni_music_api::QueueMusicData, "queueMusicData", "(Ljava/lang/Object;[SII)V");
    declareMethod(jni_music_api::StartPlaying, "startPlaying", "(Ljava/lang/Object;Ljava/lang/Object;I)V");
    declareMethod(jni_music_api::StopPlayer, "stopPlayer", "(Ljava/lang/Object;)V");
    declareMethod(jni_music_api::PausePlayer, "pausePlayer", "(Ljava/lang/Object;)V");
    declareMethod(jni_music_api::GetPosition, "getPosition", "(Ljava/lang/Object;)I");
    declareMethod(jni_music_api::SetPosition, "setPosition", "(Ljava/lang/Object;I)V");
    declareMethod(jni_music_api::SetVolume, "setVolume", "(Ljava/lang/Object;F)V");
    declareMethod(jni_music_api::IsPlaying, "isPlaying", "(Ljava/lang/Object;)Z");
    declareMethod(jni_music_api::DeletePlayer, "deletePlayer", "(Ljava/lang/Object;)V");
    declareMethod(jni_music_api::InitialPacketCount, "initialPacketCount", "(Ljava/lang/Object;)I");
}

OpaqueMusicPtr* MusicAPIAndroidImpl::createPlayer(int sampleRate) {
    AndroidOpaquePtr* ptr = new AndroidOpaquePtr();
    jobject p = env->CallObjectMethod(instance, methods[jni_music_api::CreatePlayer], sampleRate);
    if (p) {
        ptr->audioTrack = env->NewGlobalRef(p);
    } else {
        ptr->audioTrack = 0;
    }
    ptr->queuedSize = 0;

    return ptr;
}

int MusicAPIAndroidImpl::pcmBufferSize(int sampleRate) {
    return env->CallIntMethod(instance, methods[jni_music_api::PcmBufferSize], sampleRate);
}


int MusicAPIAndroidImpl::initialPacketCount(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    return env->CallIntMethod(instance, methods[jni_music_api::InitialPacketCount], ptr->audioTrack);
}

void MusicAPIAndroidImpl::queueMusicData(OpaqueMusicPtr* _ptr, short* data, int size, int sampleRate) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);

    if (!ptr->audioTrack)
        return;

    // retrieve byte[] from Java
    jshortArray b = (jshortArray) env->CallObjectMethod(instance, methods[jni_music_api::Allocate], size);
    env->SetShortArrayRegion(b, 0, size, (jshort*)data);
    delete[] data;

    env->CallVoidMethod(instance, methods[jni_music_api::QueueMusicData], ptr->audioTrack, b, size, sampleRate);
}

void MusicAPIAndroidImpl::startPlaying(OpaqueMusicPtr* _ptr, OpaqueMusicPtr* master, int offset) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

    env->CallVoidMethod(instance, methods[jni_music_api::StartPlaying],
        ptr->audioTrack,
        master ? (static_cast<AndroidOpaquePtr*>(master))->audioTrack : 0,
        offset);
     ptr->queuedSize = 0;
}

void MusicAPIAndroidImpl::stopPlayer(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

    env->CallVoidMethod(instance, methods[jni_music_api::StopPlayer], ptr->audioTrack);
}

void MusicAPIAndroidImpl::pausePlayer(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

    env->CallVoidMethod(instance, methods[jni_music_api::PausePlayer], ptr->audioTrack);
}

int MusicAPIAndroidImpl::getPosition(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return 0;

    return env->CallIntMethod(instance, methods[jni_music_api::GetPosition], ptr->audioTrack);
}

void MusicAPIAndroidImpl::setPosition(OpaqueMusicPtr* _ptr, int pos) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;
    env->CallVoidMethod(instance, methods[jni_music_api::SetPosition], ptr->audioTrack, pos);
}

void MusicAPIAndroidImpl::setVolume(OpaqueMusicPtr* _ptr, float v) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return;

    env->CallVoidMethod(instance, methods[jni_music_api::SetVolume], ptr->audioTrack, v);
}

void MusicAPIAndroidImpl::deletePlayer(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    env->CallVoidMethod(instance, methods[jni_music_api::DeletePlayer], ptr->audioTrack);
    if (ptr->audioTrack)
        env->DeleteGlobalRef(ptr->audioTrack);
    delete ptr;
}

bool MusicAPIAndroidImpl::isPlaying(OpaqueMusicPtr* _ptr) {
    AndroidOpaquePtr* ptr = static_cast<AndroidOpaquePtr*> (_ptr);
    if (!ptr->audioTrack)
        return false;
    return env->CallBooleanMethod(instance, methods[jni_music_api::IsPlaying], ptr->audioTrack);
}
