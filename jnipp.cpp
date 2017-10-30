//
// (c) Gyorgy Straub, Nuclear Heart Interactive, 2013 - 2017
//
#include "jnipp.h"
#include <cstring>
#include <cassert>

namespace jnipp
{

static JavaVM* s_javaVM = NULL;

//=============================================================================
void RegisterJavaVM(JavaVM* java_vm)
{
  s_javaVM = java_vm;
}

//=============================================================================
Env::Env()
: m_detach(false)
{
  assert(s_javaVM != nullptr); // Call RegisterJavaVM() first!
  jint result = s_javaVM->GetEnv((void**)&m_env, JNI_VERSION_1_6);
  if (JNI_EDETACHED == result)
  {
    // No JNIEnv attached to thread -- attempting to attach.
    result = s_javaVM->AttachCurrentThread((void**)&m_env, NULL);

    if (m_env)
    {
      m_detach = true;
    }
  }

  assert(JNI_OK == result); // Failed to get JNIEnv.
}

//=============================================================================
Env::Env(JNIEnv * env)
: m_detach(false),
  m_env(env)
{}

//=============================================================================
Env::~Env()
{
  if (m_detach)
  {
    s_javaVM->DetachCurrentThread();
  }
}

//=============================================================================
Class::Class(Env& e, char const * name)
: LocalRefHolder<jclass>(e, (*e)->FindClass(name))
{
  if (m_ref == 0)
  {
    jthrowable exc = (*e)->ExceptionOccurred();
    if (exc)
    {
      (*e)->ExceptionDescribe();
    }
  }
}

//=============================================================================
Class::Class(Env & e, jobject o)
: LocalRefHolder<jclass>(e, (*e)->GetObjectClass(o))
{}

//=============================================================================
jfieldID Class::GetStaticFieldId(char const * name, char const * signature)
{
  return (*m_env)->GetStaticFieldID(m_ref, name, signature);
}

//=============================================================================
jfieldID Class::GetFieldId(char const * name, char const * signature)
{
  return (*m_env)->GetFieldID(m_ref, name, signature);
}

//=============================================================================
jmethodID Class::GetStaticMethodId(char const * name, char const * signature)
{
  return (*m_env)->GetStaticMethodID(m_ref, name, signature);
}

//=============================================================================
jmethodID Class::GetMethodId(char const * name, char const * signature)
{
  return (*m_env)->GetMethodID(m_ref, name, signature);
}

//=============================================================================
Object::Object(Env& e, jobject o)
: LocalRefHolder<jobject>(e, o)
{}

//=============================================================================
size_t Object::GetArrayLength() const
{
  return (*m_env)->GetArrayLength((jarray)m_ref);
}

//=============================================================================
jobject Object::GetObjectArrayElement(size_t index)
{
  return (*m_env)->GetObjectArrayElement((jobjectArray)m_ref, index);
}

//=============================================================================
void String::ReleaseChars()
{
  if (m_releaseChars)
  {
    (*m_env)->ReleaseStringUTFChars(m_ref, m_chars);
    m_releaseChars = false;
  }
}

//=============================================================================
String::String(Env & e, char const * p, size_t len)
: LocalRefHolder<jstring>(e, (*e)->NewStringUTF(p)),
  m_chars(p),
  m_length(len == -1 ? strlen(p) : len),
  m_releaseChars(false)
{}

//=============================================================================
String::String(Env & e, jstring s, jboolean* out_copy)
: LocalRefHolder<jstring>(e, s),
  m_chars(s ? (*e)->GetStringUTFChars(s, out_copy) : nullptr),
  m_length(s ? (*e)->GetStringUTFLength(s) : 0),
  m_releaseChars(m_chars != nullptr)
{}

//=============================================================================
String::~String()
{
  ReleaseChars();
}

}
