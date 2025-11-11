#include "view.h"




//
//view

REFLEX_BEGIN_INTERNAL(ShareBox)


// Animation helper function
void Appear(GLX::Object& obj, bool value)
{
	auto from = value ? 0.0f : 1.0f;
	auto to = value ? 1.0f : 0.0f;

	auto animation = GLX::CreateOpacityAnimation(K32("appear"), from, to);
	animation->SetTime(0.25f);  // ← Changed from . to ->

	GLX::Run(obj, K32("appear"), animation);

	

	SetProperty(obj, "entered", value);
}

void ChangeSize(GLX::Object& dialog, GLX::Object& invite_section, bool expanding) {

	// Create property animation for the size property
	auto size_from = dialog.GetRect().size;
	auto invite_height = invite_section.GetRect().size.h;

	auto size_to = size_from;
	if (expanding) {
		// Growing: current + invite section height
		size_to.h = size_from.h + invite_height;
	}
	else {
		// Collapsing: current - invite section height
		size_to.h = size_from.h - invite_height;
	}

	// Create animation from current size to target size
	auto size_animation = GLX::CreateInterpolatedAnimation(
		[size_from, size_to](GLX::Object& target, float x)
		{
			auto w = Reflex::LinearInterpolate(x, size_from.w, size_to.w);
			auto h = Reflex::LinearInterpolate(x, size_from.h, size_to.h);
			GLX::SetBounds(target, K32("changesize"), {}, { w, h });
		}
	);

	GLX::Run(dialog, K32("resize"), 0.25f, size_animation);

}

class ViewImpl : public View
{
public:
	ViewImpl(Instance& instance);  

private:
	
	 // Bootstrap::View state-persistence callbacks
	virtual void OnResetState(Key32 context) override;
	virtual void OnRestoreState(Data::Archive::View& stream, Key32 context) override;
	virtual void OnStoreState(Data::Archive& stream) const override;

	// GLX::Object callbacks
	virtual bool OnEvent(GLX::Object& source, GLX::Event& e) override;
	virtual void OnSetStyle(const GLX::Style& style) override;
	virtual void OnUpdate() override;

	const TRef<Instance> instance;

	// === UI ELEMENTS ===

	// Main dialog container
	GLX::Object m_dialog;

	// Header
	GLX::Object m_share_heading;

	// Anyone section
	GLX::Object m_anyone_section;
	GLX::Object m_anyone_row;
	GLX::Object m_globe_icon;
	GLX::Object m_anyone_text_container;
	GLX::Object m_anyone_title;
	GLX::Object m_anyone_description;
	GLX::Object m_toggle_switch;
	GLX::Object m_toggle_knob;

	// URL row
	GLX::Object m_url_row;
	GLX::Object m_url_text;
	GLX::Object m_copy_icon;

	// Invite section
	GLX::Object m_invite_section;
	GLX::Object m_invite_heading;
	GLX::Object m_email_input_container;
	GLX::Object m_email_icon;
	GLX::TextArea m_email_input;
	GLX::Button m_invite_button;

	// Shared user row
	GLX::Object m_shared_user_row;
	GLX::Object m_user_avatar;
	GLX::Object m_user_info_container;
	GLX::Object m_user_name;
	GLX::Object m_user_email;
	GLX::Button m_remove_button;

	// State
	bool m_toggle_active;

	static constexpr UInt16 kChunkVersion = 1;
};
ViewImpl::ViewImpl(Instance& instance)  // ← Changed
	: View(instance, kChunkVersion, L":res:ShareBox/styles.txt"),
	instance(instance),  // ← Changed
	m_invite_button(L"Invite"),
	m_remove_button(L"Remove"),
	m_email_input(false),
	m_toggle_active(false)

{
	// Make root resizable
	Data::SetBool(SELF, GLX::kresize, true);

	// Set root flow to vertical
	GLX::SetFlow(SELF, GLX::kFlowY);

	// === DIALOG CONTAINER ===
	// Centered floating dialog
	GLX::AddFloat(SELF, m_dialog, GLX::kAlignmentCenter);
	GLX::SetFlow(m_dialog, GLX::kFlowY);

	// === SHARE HEADING ===
	GLX::AddInline(m_dialog, m_share_heading);

	// === ANYONE SECTION ===
	GLX::AddInline(m_dialog, m_anyone_section);

	// Anyone Row (horizontal flow)
	GLX::AddInline(m_anyone_section, m_anyone_row);
	GLX::SetFlow(m_anyone_row, GLX::kFlowX);

	// Globe icon
	GLX::AddInlineFlex(m_anyone_row, m_globe_icon);

	// Text container (vertical flow, centered)
	GLX::AddInlineFlex(m_anyone_row, m_anyone_text_container, GLX::kOrientationCenter);
	GLX::SetFlow(m_anyone_text_container, GLX::kFlowY);

	GLX::AddInline(m_anyone_text_container, m_anyone_title);
	GLX::AddInline(m_anyone_text_container, m_anyone_description);

	// Toggle switch (right-aligned)
	GLX::AddInline(m_anyone_row, m_toggle_switch, GLX::kOrientationFar);

	// Toggle knob inside switch
	GLX::AddInline(m_toggle_switch, m_toggle_knob);

	// === URL ROW ===
	GLX::AddInline(m_dialog, m_url_row);
	GLX::SetFlow(m_url_row, GLX::kFlowX);

	GLX::AddInline(m_url_row, m_url_text);
	GLX::AddInline(m_url_row, m_copy_icon);

	// === INVITE SECTION (GROUPED) ===
	GLX::AddInline(m_dialog, m_invite_section);
	GLX::SetFlow(m_invite_section, GLX::kFlowY);

	GLX::AddInline(m_invite_section, m_invite_heading);

	// Email input container (horizontal flow)
	GLX::AddInline(m_invite_section, m_email_input_container);
	GLX::SetFlow(m_email_input_container, GLX::kFlowX);

	GLX::AddInline(m_email_input_container, m_email_icon);
	// Get the editable content area of the TextArea
	GLX::AddInlineFlex(m_email_input_container, m_email_input);
	GLX::SetText(m_email_input.GetContent(), L"test@email.com");
	

	// Invite button (floated right)
	GLX::AddFloat(m_email_input_container, m_invite_button, GLX::kAlignmentRight);

	// === SHARED USER ROW ===
	GLX::AddInline(m_invite_section, m_shared_user_row);
	GLX::SetFlow(m_shared_user_row, GLX::kFlowX);

	GLX::AddInline(m_shared_user_row, m_user_avatar);

	// User info container (vertical flow, centered vertically)
	GLX::AddInline(m_shared_user_row, m_user_info_container, GLX::kOrientationCenter);
	GLX::SetFlow(m_user_info_container, GLX::kFlowY);

	GLX::AddInline(m_user_info_container, m_user_name);
	GLX::AddInline(m_user_info_container, m_user_email);

	// Remove button (right-aligned, flex to push it)
	GLX::AddInlineFlex(m_shared_user_row, m_remove_button, GLX::kOrientationFar);

	// Start with invite section visible (toggle off)
	GLX::SetOpacity(m_invite_section, K32("appear"), 1.0f);

}

void ViewImpl::OnResetState(Key32 context)
{
	m_toggle_active = false;
}

void ViewImpl::OnRestoreState(Data::Archive::View& stream, Key32 context)
{
	Data::Restore(stream, m_toggle_active);
}

void ViewImpl::OnStoreState(Data::Archive& stream) const
{
	Data::Store(stream, m_toggle_active);
}

bool ViewImpl::OnEvent(GLX::Object& src, GLX::Event& e)
{
	if (e.id == GLX::kMouseDown)
	{
		// Toggle switch clicked
		if (src == m_toggle_switch || src == m_toggle_knob)
		{
			m_toggle_active = !m_toggle_active;

			// Update toggle state
			GLX::SetState(m_toggle_switch, "active", m_toggle_active);
			GLX::SetState(m_toggle_knob, "active", m_toggle_active);

			// Animate invite section visibility
			// When toggle is ON (anyone can access), HIDE invite section
			// When toggle is OFF, SHOW invite section
			bool show_invite = !m_toggle_active;
			GLX::SetOpacity(m_invite_section, K32("appear"), show_invite ? 1.0f : 0.0f);

			// Pass m_dialog to enable size animation
			
			Appear(m_invite_section, show_invite);
			ChangeSize(m_dialog,m_invite_section, show_invite);
			

			// Notify instance
			instance->SetSharingEnabled(m_toggle_active);

			return true;
		}

		// Copy icon clicked
		if (src == m_copy_icon)
		{
			// Copy URL to clipboard
			instance->CopyShareLinkToClipboard();
			return true;
		}


		
		// Invite button clicked
		if (src == m_invite_button)
		{
			// Use GetContent() directly without storing
			auto email_text = GetText(m_email_input.GetContent());
			Print(output, "Email text:", email_text);
			

			// Convert to UTF-8 string for the app layer
			auto email_utf8 = Data::EncodeUTF8(email_text);
			auto email_string = Data::Unpack<CString::View>(email_utf8);
			Print(output, "Email string:", email_string);

			// Send the invite with actual email
			instance->SendInvite(email_string);

			// Clear the input after sending
			GLX::ClearText(m_email_input.GetContent());

			return true;
		}

		// Remove button clicked
		if (src == m_remove_button)
		{
			instance->RemoveSharedUser();
			return true;
		}
	}

	

	return Bootstrap::View::OnEvent(src, e);
}

void ViewImpl::OnSetStyle(const GLX::Style& style)
{
	// Main dialog
	m_dialog.SetStyle(style["ShareDialog"]);

	// Header
	m_share_heading.SetStyle(style["ShareHeading"]);

	// Anyone section
	m_anyone_section.SetStyle(style["AnyoneSection"]);
	m_anyone_row.SetStyle(style["AnyoneRow"]);
	m_globe_icon.SetStyle(style["GlobeIcon"]);
	m_anyone_text_container.SetStyle(style["AnyoneTextContainer"]);
	m_anyone_title.SetStyle(style["AnyoneTitle"]);
	m_anyone_description.SetStyle(style["AnyoneDescription"]);
	m_toggle_switch.SetStyle(style["ToggleSwitch"]);
	m_toggle_knob.SetStyle(style["ToggleKnob"]);

	// URL row
	m_url_row.SetStyle(style["URLRow"]);
	m_url_text.SetStyle(style["URLText"]);
	m_copy_icon.SetStyle(style["CopyIcon"]);

	// Invite section
	m_invite_section.SetStyle(style["InviteSection"]);
	m_invite_heading.SetStyle(style["InviteHeading"]);
	m_email_input_container.SetStyle(style["EmailInputContainer"]);
	m_email_input.SetStyle(style["EmailInput"]);
	m_email_icon.SetStyle(style["EmailIcon"]);
	m_invite_button.SetStyle(style["InviteButton"]);

	// Shared user row
	m_shared_user_row.SetStyle(style["SharedUserRow"]);
	m_user_avatar.SetStyle(style["UserAvatar"]);
	m_user_info_container.SetStyle(style["UserInfoContainer"]);
	m_user_name.SetStyle(style["UserName"]);
	m_user_email.SetStyle(style["UserEmail"]);
	m_remove_button.SetStyle(style["RemoveButton"]);

	// Update to reflect current state
	Update();
}

void ViewImpl::OnUpdate()
{
	// Update toggle state
	m_toggle_active = instance->IsSharingEnabled();
	GLX::SetState(m_toggle_switch, "active", m_toggle_active);
	GLX::SetState(m_toggle_knob, "active", m_toggle_active);

	// Update invite section visibility (without animation)
	bool show_invite = !m_toggle_active;
	GLX::SetOpacity(m_invite_heading, K32("appear"), show_invite ? 1.0f : 0.0f);
	GLX::SetOpacity(m_email_input_container, K32("appear"), show_invite ? 1.0f : 0.0f);
	GLX::SetOpacity(m_shared_user_row, K32("appear"), show_invite ? 1.0f : 0.0f);
}

REFLEX_END_INTERNAL

TRef <ShareBox::View> ShareBox::View::Create(Instance & instance)
{
	return New<ViewImpl>(instance);
}
