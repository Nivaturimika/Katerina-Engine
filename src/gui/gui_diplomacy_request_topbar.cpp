#include "demographics.hpp"
#include "gui_element_types.hpp"
#include "gui_element_templates.hpp"
#include "gui_diplomacy_request_topbar.hpp"

namespace ui {
	void diplomatic_message_topbar_button::button_action(sys::state& state) noexcept {
		auto const m = retrieve<diplomatic_message::message>(state, parent);
		//Invoke window
		auto dpw = static_cast<ui::diplomacy_request_window*>(state.ui_state.request_window);
		dpw->messages.push_back(m);
		dpw->set_visible(state, true);
		dpw->impl_on_update(state);
		state.ui_state.root->move_child_to_front(dpw);
		//Remove from listbox
		auto dmtl = static_cast<diplomatic_message_topbar_listbox*>(state.ui_state.request_topbar_listbox);
		auto it = std::remove_if(dmtl->messages.begin(), dmtl->messages.end(), [&](auto& e) { return e.from == m.from && e.to == m.to && e.type == m.type && e.when == m.when; });
		auto r = std::distance(it, dmtl->messages.end());
		dmtl->messages.erase(it, dmtl->messages.end());
		dmtl->impl_on_update(state);
	}

	void diplomatic_message_topbar_flag_button::button_action(sys::state& state) noexcept {
		if(parent) {
			auto win = static_cast<diplomatic_message_topbar_entry_window*>(parent);
			if(win->btn) {
				win->btn->button_action(state);
			}
		}
	}
}
