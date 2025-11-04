#include "view.h"




//
//view

REFLEX_BEGIN_INTERNAL(ShareBox)

class ViewImpl : public View
{
public:

	//lifetime

	ViewImpl(Instance & instance);



private:

	static constexpr UInt16 kChunkVersion = 0;	//change this to 1 to activate persistence callbacks


	//Bootstrap::View state-persistence callbacks

	virtual void OnResetState(Key32 context) override;

	virtual void OnRestoreState(Data::Archive::View & stream, Key32 context) override;

	virtual void OnStoreState(Data::Archive & stream) const override;


	//GLX::Object callbacks

	virtual bool OnEvent(GLX::Object & source, GLX::Event & e) override;

	virtual void OnSetStyle(const GLX::Style & style) override;

	virtual void OnUpdate() override;


	
	const TRef <Instance> instance;


	GLX::Object m_header;

	GLX::Popup m_popup;

	GLX::Object m_body;

	GLX::Object m_controls;

	GLX::Button m_ide;
};

ViewImpl::ViewImpl(Instance & instance)
	: View(instance, kChunkVersion, L":res:ShareBox/styles.txt"),
	instance(instance)
{
	GLX::AddInline(m_header, m_popup, GLX::kOrientationCenter);

	GLX::SetText(m_header, L"ShareBox");

	GLX::AddInline(SELF, m_header);

	LOOP(idx, instance.GetParameterValues().size) GLX::AddInline(m_controls, Bootstrap::Control::Create(instance, idx));

	GLX::AddFloat(m_body, m_controls, GLX::kAlignmentCenter);

	GLX::AddInlineFlex(SELF, m_body);

	if constexpr (REFLEX_DEBUG)
	{
		GLX::SetText(m_ide, L"Console");

		GLX::AddFloat(m_header, m_ide, GLX::kAlignmentRight);
	}
}

void ViewImpl::OnResetState(Key32 context)
{
}

void ViewImpl::OnRestoreState(Data::Archive::View & stream, Key32 context)
{
}

void ViewImpl::OnStoreState(Data::Archive & stream) const
{
}

bool ViewImpl::OnEvent(GLX::Object & src, GLX::Event & e)
{
	if (e.id == GLX::kTransaction && src.GetParent() == m_controls)
	{
		auto system = instance->instance;

		auto idx = LookupIndexOfItem(src).value;

		switch (GLX::GetTransactionStage(e))
		{
		case GLX::kTransactionBegin:
			system->BeginAutomation(idx);
			break;

		case GLX::kTransactionPerform:
			system->Automate(idx, Data::GetFloat32(e, GLX::kvalue));
			break;

		case GLX::kTransactionEnd:
		case GLX::kTransactionCancel:
			system->EndAutomation(idx);
			break;
		}
		
		return true;
	}
	else if (auto menu = GLX::GetMenu(e); menu && src == m_popup)
	{
		GLX::BindClick(menu->AddItem(L"New"), [this]()
		{
			instance->session->Reset();
		});

		GLX::BindClick(menu->AddItem(L"Open"), [this]()
		{
			if (auto path = GLX::ShowFileDialog(false, { Instance::kFileExt }, instance->GetFilename()))
			{
				instance->Open(path);
			}
		});

		GLX::BindClick(menu->AddItem(L"Save"), [this]()
		{
			if (auto path = GLX::ShowFileDialog(true, { Instance::kFileExt }, instance->GetFilename()))
			{
				instance->Save(path);
			}
		});

		return true;
	}
	else if (e.id == GLX::kMouseDown)
	{
		if constexpr (REFLEX_DEBUG)
		{
			if (src == m_ide)
			{
				Bootstrap::global->EnableIde(!Bootstrap::global->IdeEnabled());

				return true;
			}
		}
	}
	
	return Bootstrap::View::OnEvent(src, e);
}

void ViewImpl::OnSetStyle(const GLX::Style & style)
{
	auto header_style = style["Header"];

	auto control_style = style["Control"];

	m_body.SetStyle(style["Body"]);

	m_header.SetStyle(header_style);

	m_popup.SetStyle(header_style["Popup"]);

	FOREACH(i, m_controls) i.SetStyle(control_style);

	m_ide.SetStyle(style["Button"]);
}

void ViewImpl::OnUpdate()
{
	//something changed on instance

	WString title;

	if (auto filename = instance->GetFilename())
	{
		title = File::SplitExtension(File::SplitFilename(filename).b).a;
	}
	else
	{
		title = L"New";
	}

	if (instance->IsEdited()) title.Append(L" *");

	GLX::SetText(m_header, title);

	if constexpr (REFLEX_DEBUG) GLX::Select(m_ide, Bootstrap::global->IdeEnabled());
}

REFLEX_END_INTERNAL

TRef <ShareBox::View> ShareBox::View::Create(Instance & instance)
{
	return New<ViewImpl>(instance);
}
