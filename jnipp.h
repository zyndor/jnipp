#pragma once
//
// (c) Gyorgy Straub, Nuclear Heart Interactive, 2013 - 2017
//
#include "jni.h"

namespace jnipp
{

//=============================================================================
void  RegisterJavaVM(JavaVM* java_vm);

//=============================================================================
// Wraps a JNI environment with the RAII required to not crash the VM.
// NOTE: This is best used as a function local; it's ok (fantastic even)
// if the function is a thread. Definitely don't pass it across threads.
class Env
{
public:
  Env();
  explicit Env(JNIEnv* env);
  ~Env();

  JNIEnv* Ptr() const  // no ownership transfer
  {
    return **this;
  }

  JNIEnv* operator*() const  // no ownership transfer
  {
    return m_env;
  }

private:
  Env(const Env& other);
  Env& operator=(const Env& other);

  bool m_detach;
  JNIEnv* m_env;
};

//=============================================================================
// Holder for JNI objects that may be accessed, compared against null and -
// implemented by the concrete, derived type - disposed of.
template <typename T>
class Holder
{
protected:
  Holder(T ref)
  : m_ref(ref)
  {}

  mutable T m_ref;

public:
  operator T() const  // no ownership transfer
  {
    return m_ref;
  }

  operator bool() const
  {
    return m_ref != nullptr;
  }

private:
  Holder(Holder const& other);
  Holder& operator=(Holder const& other);
};

//=============================================================================
// A local reference. Deleted on destruction.
template <typename T>
class LocalRefHolder : public Holder<T>
{
protected:
  Env& m_env;

public:
  LocalRefHolder(Env& e, T ref)
  : Holder<T>(ref),
    m_env(e)
  {}

  ~LocalRefHolder()
  {
    if (Holder<T>::m_ref != 0)
    {
      m_env.Ptr()->DeleteLocalRef(Holder<T>::m_ref);
    }
  }

  T Release() // ownership transfer
  {
    T temp = Holder<T>::m_ref;
    Holder<T>::m_ref = 0;
    return temp;
  }

  T NewGlobalRef() const  // ownership transfer
  {
    return (T)m_env.Ptr()->NewGlobalRef(Holder<T>::m_ref);
  }
};

//=============================================================================
// A local jclass. Deleted on destruction.
class Class: public LocalRefHolder<jclass>
{
public:
  Class(Env& e, char const* name);
  Class(Env& e, jobject o);  // no onwership transfer

  jfieldID GetStaticFieldId(char const* name, char const* signature);
  jfieldID GetFieldId(char const* name, char const* signature);

  jmethodID GetStaticMethodId(char const* name, char const* signature);
  jmethodID GetMethodId(char const* name, char const* signature);

private:
  operator jobject();  // disable
};

//=============================================================================
// A local jobject reference. Deleted on destruction.
class Object : public LocalRefHolder<jobject>
{
public:
  Object(Env& e, jobject o);

  // Arrays only, obviously.
  size_t GetArrayLength() const;
  jobject GetObjectArrayElement(size_t index);

  template <typename T>
  T Get(jfieldID field) const
  {
    T value;
    Get(field, value);
    return value;
  }

private:
  void Get(jfieldID field, jint& value)
  {
    value = m_env.Ptr()->GetIntField(m_ref, field);
  }
};

//=============================================================================
// A local jstring reference with chars. Deleted on destruction.
// Its usage is twofold: 1, for creating jstrings from const char*s and
// 2, for holding jstrings and their chars.
class String : public LocalRefHolder<jstring>
{
  char const* m_chars;
  size_t m_length;
  bool m_releaseChars;

  void ReleaseChars();

public:
  String(Env& e, char const* p, size_t len = -1);
  String(Env& e, jstring s, jboolean* out_copy = nullptr);  // ownership transfer
  ~String();
  
  const char* Chars() const { return m_chars; }
  size_t Length() const { return m_length; }
  
  jstring Release()  // ownership transfer
  {
    ReleaseChars();
    return LocalRefHolder<jstring>::Release();
  }
};

} // JNI

