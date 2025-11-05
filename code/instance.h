#pragma once

#include "reflex_bootstrap/audioplugin.h"




//
//declarations

REFLEX_NS(ShareBox)

struct Instance : public Bootstrap::AudioPlugin
{
public:

	REFLEX_OBJECT(Instance, Bootstrap::AudioPlugin);

	static constexpr WString::View kFileExt = L"ShareBox";



	//config

	static System::AudioPlugin::Configuration::Class MakeClass();

	static TRef <ParamDefs> CreateParamDefs();



	//ctr

	static TRef <Instance> Create(System::AudioPlugin & instance);

	// Add these methods:
	virtual void SetSharingEnabled(bool enabled) = 0;
	virtual bool IsSharingEnabled() const = 0;
	virtual void CopyShareLinkToClipboard() = 0;
	virtual void SendInvite(const CString& email) = 0;
	virtual void RemoveSharedUser() = 0;



protected:

	using Bootstrap::AudioPlugin::AudioPlugin;
};

extern Debug::Output output;

REFLEX_END
