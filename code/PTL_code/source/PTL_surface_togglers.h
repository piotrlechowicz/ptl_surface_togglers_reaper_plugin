#pragma once

/**
 * Action parent interface that has action's name and id
 */
class Action {
public:
	Action(std::string actionName, 
				std::string actionId);
	std::string& getActionName();
	std::string& getActionId();
private:
	/* action name and id in actions list */
	std::string actionName;
	std::string actionId;
};


/**
 * Simple button which only toggles interial state (without any action).
 * It is used to light buttons on surface if no reaper state is associated with it 
 * (e.g. when navigating through zones).
 */
class ToggleStateButton : public Action {
public:
	ToggleStateButton(
		std::string extStateKey,
		std::string actionName,
		std::string actionId);
	
	void setState(action_entry& act, bool state);
	void toggleState(action_entry& act);
	void setStateOff(action_entry& act);

	/* section and key under which state is stored */
	std::string extStateSection = "PTL_surface";
	std::string extStateKey;

};

class ZoomButton : public ToggleStateButton {
public:
	ZoomButton();
};

class NoMotorsButton : public ToggleStateButton {
public:
	NoMotorsButton();
};

class SendsButton : public ToggleStateButton {
public:
	SendsButton();
};

class FlipButton : public ToggleStateButton {
public:
	FlipButton();
};


class TurnOffOthersButton : public Action {
public:
	TurnOffOthersButton(std::string actionName, std::string actionId);

	void apply(action_entry& act);
	void setButtonsToToggleOff(std::vector<ToggleStateButton*> buttons);
private:
	std::vector<ToggleStateButton*> buttonsToToggleOff;
};

class GoHomeButton : public TurnOffOthersButton {
public:
	GoHomeButton();
};



class PTLSurfaceButtons {
public:
	PTLSurfaceButtons();

	GoHomeButton goHomeButton;

	ZoomButton zoomButton;
	NoMotorsButton noMotorsButton;
	SendsButton sendsButton;
	FlipButton flipButton;

	
private:
	std::vector<ToggleStateButton*> homeButtonToggleOfButtons{&zoomButton, &noMotorsButton, &sendsButton, &flipButton};
};
