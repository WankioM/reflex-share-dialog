#include "instance.h"
#include "view.h"




//
//entrypoint

TRef <Object> System::AudioPlugin::OnStart(const CString::View & cmdline, Configuration & config)
{
	return Bootstrap::StartAudioPluginWithView <::ShareBox::Instance,::ShareBox::View>
	(
		config,
		"UI",
		"ShareBox",
		K32("ShareBox"),
		__FILE__
	);
}
