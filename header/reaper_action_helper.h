#pragma once

enum toggle_state { CannotToggle, ToggleOff, ToggleOn };

// Little C++ class to deal with the actions
class action_entry : public NoCopyNoMove {
public:
	action_entry(std::string description, std::string idstring, toggle_state togst, std::function<void(action_entry&)> func);
	// These are not private and behind getters/setters because we assume people
	// know what they are doing ;)
	int m_command_id = 0;
	gaccel_register_t m_accel_reg;
	std::function<void(action_entry&)> m_func;
	std::string m_desc;
	std::string m_id_string;
	toggle_state m_togglestate = CannotToggle;
	int m_cycle_state = 0;
	int m_ex_val = 0;
	int m_ex_valhw = 0;
	int m_ex_relmode = 0;
	double m_ex_valhw_parsed = 0.0;
	void* m_userdata = nullptr;
};

bool hookCommandProcEx(KbdSectionInfo *sec, int command, int val, int valhw, int relmode, HWND hwnd);
int toggleActionCallback(int command_id);

std::shared_ptr<action_entry> add_action(std::string name, std::string id, toggle_state togst, std::function<void(action_entry&)> f);

action_entry* get_action_by_id(std::string id);

