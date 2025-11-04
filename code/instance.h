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



protected:

	using Bootstrap::AudioPlugin::AudioPlugin;
};

extern Debug::Output output;

REFLEX_END
