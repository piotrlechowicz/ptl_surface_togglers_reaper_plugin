#include "utilfuncs.h"
#include "reaper_action_helper.h"
#include "reaper_plugin/reaper_plugin_functions.h"
#include <string>

#include "PTL_surface_togglers.h"

Action::Action(std::string actionName, std::string actionId) : actionName(actionName), actionId(actionId) {}

std::string& Action::getActionName() {
	return actionName;
}

std::string& Action::getActionId() {
	return actionId;
}

ToggleStateButton::ToggleStateButton(
	std::string extStateKey,
	std::string actionName,
	std::string actionId
	) : Action(actionName, actionId), extStateKey(extStateKey) {}

void ToggleStateButton::setState(action_entry& act, bool state) {
	if (state == true) {
		act.m_togglestate = ToggleOn;
	} else {
		act.m_togglestate = ToggleOff;
	}
	char buf[8];
	// the REAPER api for ini-files only deals with strings, so form a string from the action
	// toggle state number.
	int toggletemp = 0;
	if (act.m_togglestate == ToggleOn)
		toggletemp = 1;
	sprintf(buf, "%d", toggletemp);
	SetExtState(this->extStateSection.c_str(), this->extStateKey.c_str(), buf, true);
}

void ToggleStateButton::toggleState(action_entry& act) {
	if (act.m_togglestate == ToggleOff) {
		this->setState(act, true);
	} else {
		this->setState(act, false);
	}
}

void ToggleStateButton::setStateOff(action_entry& act) {
	this->setState(act, false);
}

TurnOffOthersButton::TurnOffOthersButton(std::string actionName, std::string actionId) : Action(actionName, actionId) {}



void TurnOffOthersButton::setButtonsToToggleOff(std::vector<ToggleStateButton*> buttons) {
	this->buttonsToToggleOff = buttons;
}


void TurnOffOthersButton::apply(action_entry& act) {
	for (std::vector<ToggleStateButton*>::iterator it = this->buttonsToToggleOff.begin(); it != this->buttonsToToggleOff.end(); ++it) {
		action_entry* toggling_action = get_action_by_id((*it)->getActionId());
		if (toggling_action != nullptr) {
			(*it)->setStateOff(*toggling_action);
		}
	}
}



GoHomeButton::GoHomeButton() : TurnOffOthersButton("PTL: surface - toggle home button", "PTL_surface_home_button") {}

ZoomButton::ZoomButton() : ToggleStateButton("toggle_zoom_mode", "PTL: surface - toggle zoom mode", "PTL_surface_toggle_zoom_mode") {}
NoMotorsButton::NoMotorsButton() : ToggleStateButton("toggle_no_motors_mode", "PTL: surface - toggle no motors mode", "PTL_surface_toggle_no_motors_mode") {}
SendsButton::SendsButton() : ToggleStateButton("toggle_sends_mode", "PTL: surface - toggle sends mode", "PTL_surface_toggle_sends_mode") {}
FlipButton::FlipButton() : ToggleStateButton("toggle_flip_button", "PTL: surface - toggle flip mode", "PTL_surface_toggle_flip_mode") {}




PTLSurfaceButtons::PTLSurfaceButtons() {
	this->goHomeButton.setButtonsToToggleOff(homeButtonToggleOfButtons);
}