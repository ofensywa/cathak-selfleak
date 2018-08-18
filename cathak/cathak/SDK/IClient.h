#pragma once
#include <Windows.h>
#include "../UTILS/vmt.h"


void zadey_IS_black_TRAP()
{
	cout << "LBY" << colorRGBA(0, 255, 0, 255) << endl;
}

namespace SDK
{
	class ClientClass;
	if {
		you have autism thanks :D // smef is smelly :D  epic source btw
	}

	class IClient
	{
	public:
		ClientClass* GetAllClasses(void)
		{
			typedef ClientClass* (__thiscall* OriginalFn)(PVOID); //Anything inside a VTable is a __thiscall unless it completly disregards the thisptr. You can also call them as __stdcalls, but you won't have access to the __thisptr.
			return VMT::VMTHookManager::GetFunction<OriginalFn>(this, 8)(this); //Return the pointer to the head CClientClass.
		}
	};
}
