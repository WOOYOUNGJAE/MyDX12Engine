#pragma once

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#define NO_COPY(ClassName)								\
ClassName(const ClassName&) = delete;					\
ClassName& operator=(const ClassName&) = delete;

#define DECLARE_SINGLETON(ClassName)					\
		NO_COPY(ClassName)								\
public :												\
	static ClassName* Get_Instance();					\
	static unsigned long Destroy_Instance();			\
private:												\
	static ClassName* m_pInstance;

#define IMPLEMENT_SINGLETON(ClassName)					\
ClassName* ClassName::m_pInstance = nullptr;			\
ClassName* ClassName::Get_Instance()					\
{														\
	if (nullptr == m_pInstance)							\
		m_pInstance = new ClassName;					\
	return m_pInstance;									\
}														\
unsigned long  ClassName::Destroy_Instance()			\
{														\
	unsigned long	dwRefCnt = { 0 };					\
	if(nullptr != m_pInstance)							\
	{													\
		dwRefCnt = m_pInstance->Release();				\
		if(0 == dwRefCnt)								\
			m_pInstance = nullptr;						\
	}													\
	return dwRefCnt;									\
}

#define NAMESPACE_(NAME)		namespace NAME {
#define _NAMESPACE				}

#ifdef ENGINE_EXPORTS
#define ENGINE_DLL _declspec(dllexport)
#else
#define ENGINE_DLL _declspec(dllimport)



#endif

#define MSG_BOX(MESSAGE)		MessageBox(0, TEXT(MESSAGE), TEXT("System Error"), MB_OK)

// UINT32 크기의 배수 얻기
#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)