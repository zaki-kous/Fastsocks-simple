//
// Created by 朱乾 on 17/3/26.
//

#include "android_jni.h"

jvalue __JNI_CallMethodByName(
        JNIEnv* env,
        jobject obj,
        const char* name,
        const char* descriptor,
        va_list args) {
    jclass clazz;
    jmethodID mid;
    jvalue result;
    memset(&result, 0 , sizeof(result));

    if (env->EnsureLocalCapacity(2) == JNI_OK) {
        clazz = env->GetObjectClass(obj);

        mid = env->GetMethodID(clazz, name,
                               descriptor);

        if (mid) {
            const char* p = descriptor;

            /* skip over argument types to find out the
             return type */
            while (*p != ')')
                p++;

            /* skip ')' */
            p++;

            switch (*p) {
                case 'V':
                    env->CallVoidMethodV(obj, mid, args);
                    break;

                case '[':
                case 'L':
                    result.l = env->CallObjectMethodV(obj, mid, args);
                    break;

                case 'Z':
                    result.z = env->CallBooleanMethodV(obj, mid, args);
                    break;

                case 'B':
                    result.b = env->CallByteMethodV(obj, mid, args);
                    break;

                case 'C':
                    result.c = env->CallCharMethodV(obj, mid, args);
                    break;

                case 'S':
                    result.s = env->CallShortMethodV(obj, mid, args);
                    break;

                case 'I':
                    result.i = env->CallIntMethodV(obj, mid, args);
                    break;

                case 'J':
                    result.j = env->CallLongMethodV(obj, mid, args);
                    break;

                case 'F':
                    result.f = env->CallFloatMethodV(obj, mid, args);
                    break;

                case 'D':
                    result.d = env->CallDoubleMethodV(obj, mid, args);
                    break;

                default:
                    env->FatalError("illegal descriptor");
                    break;
            }

        }

        env->DeleteLocalRef(clazz);
    }

    return result;
}

jvalue JNI_CallMethodByName(
        JNIEnv* _env,
        jobject obj,
        const char* _name,
        const char* _descriptor, ...) {

    va_list args;
    va_start(args, _descriptor);

    jvalue result =	__JNI_CallMethodByName( _env, obj, _name, _descriptor, args);

    va_end(args);
    return result;
}

jvalue __JNI_CallStaticMethodByName(
        JNIEnv* _env,
        jclass _clazz,
        const char* _name,
        const char* _descriptor,
        va_list args) {

    jmethodID mid;
    jvalue result;
    memset(&result, 0 , sizeof(result));

    mid = _env->GetStaticMethodID(_clazz, _name, _descriptor);

    if (mid) {
        const char* p = _descriptor;

        /* skip over argument types to find out the
         return type */
        while (*p != ')')
            p++;

        /* skip ')' */
        p++;

        switch (*p) {
            case 'V':
                _env->CallStaticVoidMethodV(_clazz, mid, args);
                break;

            case '[':
            case 'L':
                result.l = _env->CallStaticObjectMethodV(_clazz, mid, args);
                break;

            case 'Z':
                result.z = _env->CallStaticBooleanMethodV(_clazz, mid, args);
                break;

            case 'B':
                result.b = _env->CallStaticByteMethodV(_clazz, mid, args);
                break;

            case 'C':
                result.c = _env->CallStaticCharMethodV(_clazz, mid, args);
                break;

            case 'S':
                result.s = _env->CallStaticShortMethodV(_clazz, mid, args);
                break;

            case 'I':
                result.i = _env->CallStaticIntMethodV(_clazz, mid, args);
                break;

            case 'J':
                result.j = _env->CallStaticLongMethodV(_clazz, mid, args);
                break;

            case 'F':
                result.f = _env->CallStaticFloatMethodV(_clazz, mid, args);
                break;

            case 'D':
                result.d = _env->CallStaticDoubleMethodV(_clazz, mid, args);
                break;

            default:
                _env->FatalError("illegal _descriptor");
                break;
        }
    }

    return result;
}

jvalue JNI_CallStaticMethodByName(
        JNIEnv* _env,
        jclass _clazz,
        const char* _name,
        const char* _descriptor, ...) {
    va_list args;

    va_start(args, _descriptor);
    jvalue result = __JNI_CallStaticMethodByName(_env, _clazz, _name, _descriptor, args);
    va_end(args);

    return result;
}

jvalue JNI_CallStaticMethodByName(
        JNIEnv* _env,
        const char* className,
        const char* _name,
        const char* _descriptor, ...) {
    jclass _clazz = _env->FindClass(className);

    va_list args;
    va_start(args, _descriptor);
    jvalue result = __JNI_CallStaticMethodByName(_env, _clazz, _name, _descriptor, args);
    va_end(args);

    return result;
}

jvalue JNI_GetStaticField(JNIEnv* _env, jclass _clazz, const char* _name, const char* sig) {

    jvalue result;
    memset(&result, 0 , sizeof(result));

    jfieldID fid = _env->GetStaticFieldID(_clazz, _name, sig);

    if (NULL == fid) {
        return result;
    }

    switch (*sig) {
        case '[':
        case 'L':
            result.l = _env->GetStaticObjectField(_clazz, fid);
            break;

        case 'Z':
            result.z = _env->GetStaticBooleanField(_clazz, fid);
            break;

        case 'B':
            result.b = _env->GetStaticByteField(_clazz, fid);
            break;

        case 'C':
            result.c = _env->GetStaticCharField(_clazz, fid);
            break;

        case 'S':
            result.s = _env->GetStaticShortField(_clazz, fid);
            break;

        case 'I':
            result.i = _env->GetStaticIntField(_clazz, fid);
            break;

        case 'J':
            result.j = _env->GetStaticLongField(_clazz, fid);
            break;

        case 'F':
            result.f = _env->GetStaticFloatField(_clazz, fid);
            break;

        case 'D':
            result.d = _env->GetStaticDoubleField(_clazz, fid);
            break;

        default:
            _env->FatalError("illegal _descriptor");
            break;
    }

    return result;
}

jvalue JNI_GetField(JNIEnv* _env, jobject obj, const char* _name, const char* sig) {
    jvalue result;
    memset(&result, 0 , sizeof(result));

    if (_env->ExceptionOccurred()) {
        return result;
    }

    jclass _clazz = _env->GetObjectClass(obj);
    jfieldID fid = _env->GetFieldID(_clazz, _name, sig);
    _env->DeleteLocalRef(_clazz);

    if (NULL == fid) {
        return result;
    }

    switch (*sig) {
        case '[':
        case 'L':
            result.l = _env->GetObjectField(obj, fid);
            break;

        case 'Z':
            result.z = _env->GetBooleanField(obj, fid);
            break;

        case 'B':
            result.b = _env->GetByteField(obj, fid);
            break;

        case 'C':
            result.c = _env->GetCharField(obj, fid);
            break;

        case 'S':
            result.s = _env->GetShortField(obj, fid);
            break;

        case 'I':
            result.i = _env->GetIntField(obj, fid);
            break;

        case 'J':
            result.j = _env->GetLongField(obj, fid);
            break;

        case 'F':
            result.f = _env->GetFloatField(obj, fid);
            break;

        case 'D':
            result.d = _env->GetDoubleField(obj, fid);
            break;

        default:
            _env->FatalError("illegal _descriptor");
            break;
    }

    return result;
}

// char* to jstring
jstring JNI_Chars2Jstring(JNIEnv* _env, const char* pat) {
    jclass str_class = _env->FindClass("java/lang/String");
    jmethodID ctorID = _env->GetMethodID(str_class, "<init>", "([BLjava/lang/String;)V");

    jbyteArray bytes;

    if (pat != NULL) {
        bytes = _env->NewByteArray((jsize)strlen(pat));
        _env->SetByteArrayRegion(bytes, 0, (jsize)strlen(pat), (jbyte*) pat);
    } else {
        bytes = _env->NewByteArray(1);
        char ch[1] =
                { 0 };
        _env->SetByteArrayRegion(bytes, 0, 1, (jbyte*) ch);
    }

    jstring encoding = _env->NewStringUTF("utf-8");

    jstring jstr = (jstring) _env->NewObject(str_class, ctorID, bytes, encoding);
    _env->DeleteLocalRef(bytes);
    _env->DeleteLocalRef(encoding);

    return jstr;
}


jbyteArray JNI_Buffer2JbyteArray(JNIEnv* _env, const void* _buffer, size_t _length) {
    unsigned int len = _length;

    if (len == 0) {
        return NULL;
    }

    jbyteArray bytes = _env->NewByteArray((jsize)len);
    _env->SetByteArrayRegion(bytes, 0, (jsize)len, (jbyte*) _buffer);
    return bytes;
}

void JNI_FreeJbyteArray(JNIEnv* _env, jbyteArray bytes) {
    _env->DeleteLocalRef(bytes);
}

void JNI_FreeJstring(JNIEnv* _env, jstring str) {
    _env->DeleteLocalRef(str);
}
