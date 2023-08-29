#include <cassert>
#include "http/java_net_HttpURLConnection.h"
#include "android/JavaVM.h"

using namespace java::net;

void HttpURLConnection::disconnect()
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	env->CallVoidMethod(m_this, classInfo.disconnect);
	Framework::CJavaVM::CheckException(env);
}

jobject HttpURLConnection::getErrorStream()
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	jobject result = env->CallObjectMethod(m_this, classInfo.getErrorStream);
	Framework::CJavaVM::CheckException(env);
	assert(result != NULL);
	return result;
}

jstring HttpURLConnection::getHeaderField(jint index)
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	auto result = static_cast<jstring>(env->CallObjectMethod(m_this, classInfo.getHeaderField, index));
	Framework::CJavaVM::CheckException(env);
	return result;
}

jstring HttpURLConnection::getHeaderFieldKey(jint index)
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	auto result = static_cast<jstring>(env->CallObjectMethod(m_this, classInfo.getHeaderFieldKey, index));
	Framework::CJavaVM::CheckException(env);
	return result;
}

jobject HttpURLConnection::getInputStream()
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	jobject result = env->CallObjectMethod(m_this, classInfo.getInputStream);
	Framework::CJavaVM::CheckException(env);
	assert(result != NULL);
	return result;
}

jobject HttpURLConnection::getOutputStream()
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	jobject result = env->CallObjectMethod(m_this, classInfo.getOutputStream);
	Framework::CJavaVM::CheckException(env);
	assert(result != NULL);
	return result;
}

jint HttpURLConnection::getResponseCode()
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	auto result = env->CallIntMethod(m_this, classInfo.getResponseCode);
	Framework::CJavaVM::CheckException(env);
	return result;
}

void HttpURLConnection::setRequestMethod(jstring method)
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	env->CallVoidMethod(m_this, classInfo.setRequestMethod, method);
	Framework::CJavaVM::CheckException(env);
}

void HttpURLConnection::setRequestProperty(jstring key, jstring value)
{
	auto env = Framework::CJavaVM::GetEnv();
	const auto& classInfo = ClassInfo::GetInstance();
	env->CallVoidMethod(m_this, classInfo.setRequestProperty, key, value);
	Framework::CJavaVM::CheckException(env);
}

void HttpURLConnection_ClassInfo::PrepareClassInfo()
{
	auto env = Framework::CJavaVM::GetEnv();
	
	jclass tmpClazz = env->FindClass("java/net/HttpURLConnection");
	Framework::CJavaVM::CheckException(env);
	assert(tmpClazz != NULL);
	clazz = reinterpret_cast<jclass>(env->NewGlobalRef(tmpClazz));
	assert(clazz != NULL);
	
	disconnect = env->GetMethodID(clazz, "disconnect", "()V");
	Framework::CJavaVM::CheckException(env);
	assert(disconnect != NULL);
	
	getErrorStream = env->GetMethodID(clazz, "getErrorStream", "()Ljava/io/InputStream;");
	Framework::CJavaVM::CheckException(env);
	assert(getErrorStream != NULL);
	
	getHeaderField = env->GetMethodID(clazz, "getHeaderField", "(I)Ljava/lang/String;");
	Framework::CJavaVM::CheckException(env);
	assert(getHeaderField != NULL);
	
	getHeaderFieldKey = env->GetMethodID(clazz, "getHeaderFieldKey", "(I)Ljava/lang/String;");
	Framework::CJavaVM::CheckException(env);
	assert(getHeaderFieldKey != NULL);
	
	getInputStream = env->GetMethodID(clazz, "getInputStream", "()Ljava/io/InputStream;");
	Framework::CJavaVM::CheckException(env);
	assert(getInputStream != NULL);
	
	getOutputStream = env->GetMethodID(clazz, "getOutputStream", "()Ljava/io/OutputStream;");
	Framework::CJavaVM::CheckException(env);
	assert(getOutputStream != NULL);
	
	getResponseCode = env->GetMethodID(clazz, "getResponseCode", "()I");
	Framework::CJavaVM::CheckException(env);
	assert(getResponseCode != NULL);
	
	setRequestMethod = env->GetMethodID(clazz, "setRequestMethod", "(Ljava/lang/String;)V");
	Framework::CJavaVM::CheckException(env);
	assert(setRequestMethod != NULL);
	
	setRequestProperty = env->GetMethodID(clazz, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
	Framework::CJavaVM::CheckException(env);
	assert(setRequestProperty != NULL);
}
