#include "instance.h"




//
//

REFLEX_BEGIN_INTERNAL(ShareBox)

struct InstanceImpl :
	public Instance,
	public Bootstrap::Streamable
{
	static constexpr UInt16 kChunkVersion = 0;	//change this to 1 to activate persistence callbacks

	InstanceImpl(System::AudioPlugin & audioplugin)
		: Instance(K32("ShareBox"), audioplugin),	//first parameter is 4 byte header for the file format
		Bootstrap::Streamable(session, K32("custom"), kChunkVersion),
		m_monitor(Cast<Reflex::State>(SELF)),
		m_amp(0.0f),
		m_inc(0.0f),
		m_phase(0.0f)
	{
		Print(output, "ShareBox Instance constructed");
	}

	//Bootstrap::Streamable callbacks
	//parameters are stored automatically in baseclass, but additional/custom data can be stored here
	//to enable these callbacks, set kChunkVersion to 1

	virtual void OnReset(Key32 context) override
	{
	}

	virtual void OnRestore(Data::Archive::View & stream, Key32 context) override
	{
	}

	virtual void OnStore(Data::Archive & stream) const override
	{
	}

	// View interface methods
	virtual void SetSharingEnabled(bool enabled) override
	{
		m_sharing_enabled = enabled;
		Notify(true);  // Update view
	}

	virtual bool IsSharingEnabled() const override
	{
		return m_sharing_enabled;
	}

	virtual void CopyShareLinkToClipboard() override
	{
		Print(output, "Copy link to clipboard");
		// TODO: Implement clipboard functionality
	}

	virtual void SendInvite(const CString& email) override
	{
		Print(output, "Send invite to:", email);
		// TODO: Implement invite functionality
	}

	virtual void RemoveSharedUser() override
	{
		Print(output, "Remove shared user");
		// TODO: Implement remove user functionality
	}

	//Bootstrap::AudioPlugin callbacks
	//implement these callbacks to process/generate audio

	virtual bool OnPrepareProcessing(UInt maxbuffersize, Float32 sr, UInt ninput, UInt noutput) override
	{
		m_sr = sr;

		return noutput > 0;	//allow processing if at least 1 output
	}

	virtual void OnProcessRt(UInt samples, const System::AudioPlugin::EventBuffer & events, Array <System::AudioPlugin::Event> & eventsout, const ArrayView <const Float*> & inputs, const ArrayView <Float*> & outputs) override
	{
		//YOUR DSP HERE...

		if (m_monitor.Poll())
		{
			//parameters changed

			auto params = GetParameterValues();

			switch (params[0].ivalue)
			{
			case 1:
				m_generatefn = &InstanceImpl::GenerateWaveform<1>;
				break;

			case 0:
				m_generatefn = &InstanceImpl::GenerateWaveform<0>;
				break;

			default:
				m_generatefn = [](UInt samples, Float amp, Float inc, Float phase, Float * pout) { return 0.0f; };
				break;
			}

			auto f = params[1].fvalue;

			m_inc = f / m_sr;

			m_amp = params[2].fvalue;

			m_fxfn = True(params[3].ivalue) ? &InstanceImpl::ProcessFX : [](UInt samples, Float * pinout) {};

			Print(output, "Updating", "amp:", m_amp, "freq:", f);
		}

		auto l = outputs[0];

		m_phase = m_generatefn(samples, m_amp, m_inc, m_phase, l);

		m_fxfn(samples, l);

		auto rest = Splice(outputs, 1).b;

		FOREACH(i, rest)
		{
			MemCopy(l, i, samples * sizeof(Float32));
		}
	}

	template <UInt SHAPE> static Float32 GenerateWaveform(UInt samples, Float amp, Float inc, Float phase, Float * pout)
	{
		//a 'naive' (non anti-aliased) waveform for this simple example

		LOOP_PTR(pout, psample, samples)
		{
			phase += inc;

			if (phase > 1.0f) phase -= 1.0f;

			Float32 value;

			if constexpr (SHAPE)
			{
				value = Float(phase > 0.5f);
			}
			else
			{
				value = Sin(Reflex::k2Pif * ((phase * 2.0f) - 1.0f));
			}

			value *= amp;

			*psample = value;
		}

		return phase;
	}

	static void ProcessFX(UInt samples, Float * pinout)
	{
		//simple bit crush for example

		Float32 q = 1.0f / 16.0f;

		LOOP_PTR(pinout, psample, samples)
		{
			*psample = Reflex::Quantise(*psample, q);
		}
	}


	
	State::Monitor m_monitor;

	//your state and data here...

	Float m_sr;

	Float m_amp, m_inc, m_phase;

	decltype (&InstanceImpl::GenerateWaveform<0>) m_generatefn;

	decltype (&InstanceImpl::ProcessFX) m_fxfn;

	bool m_sharing_enabled;
};

REFLEX_END_INTERNAL

System::AudioPlugin::Configuration::Class ShareBox::Instance::MakeClass()
{
	typedef System::AudioPlugin::Configuration::Class Class;

	Class cls;

	cls.channels_io = { 2, 2 };

	cls.nparam = 4;

	cls.vst2.uid = Reflex::Detail::MergeHashes(K32("UI"), K32("ShareBox"));

	cls.vst2.instrument = false;

	cls.vst3.uid = { K64("UI"), K64("ShareBox") };

	cls.vst3.category = Class::VST3::kFx;

	cls.audiounit.company_4cc = CC32("f659");

	cls.audiounit.uid_4cc = CC32("747a");

	cls.audiounit.type_4cc = Class::AudioUnit::kFx;

	return cls;
}

TRef <Bootstrap::AudioPlugin::ParamDefs> ShareBox::Instance::CreateParamDefs()
{
	typedef Bootstrap::ParamInfo ParamInfo;

	auto paramdefs = New<Bootstrap::AudioPlugin::ParamDefs>();

	paramdefs->value.Append
	({
		{ K32("Mode"), ParamInfo::CreateEnum("Mode", { "Sine", "Square" }, 0) },
		{ K32("Freq"), ParamInfo::CreateReal("Freq", 100.0f, 1000.0f, 0.0f, 0.0f) },
		{ K32("Amp"), ParamInfo::CreateReal("Amp", 0.0f, 1.0f, 0.0f, 0.0f) },
		{ K32("FX"), ParamInfo::CreateBool("FX", false) },
	});

	return paramdefs;
}

TRef <ShareBox::Instance> ShareBox::Instance::Create(System::AudioPlugin & instance)
{
	return New<InstanceImpl>(instance);
}

Reflex::Debug::Output ShareBox::output("ShareBox");
