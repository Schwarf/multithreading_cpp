//
// Created by andreas on 24.04.23.
//

#ifndef HAZARD_POINTER_H
#define HAZARD_POINTER_H
#include <atomic>
#include <vector>
#include <thread>

template<typename T>
class HazardPointer {
public:
	HazardPointer() : hazard_{nullptr} {}

	std::atomic<T*>& Get() {
		return hazard_;
	}

private:
	std::atomic<T*> hazard_;
};

template<typename T>
class HazardPointerManager {
public:
	HazardPointerManager() : max_hp_{std::thread::hardware_concurrency()}, hp_{max_hp_}, retired_{nullptr} {}

	~HazardPointerManager() {
		for (int i = 0; i < max_hp_; ++i) {
			delete hp_[i];
		}

		auto current = retired_.load();
		while (current) {
			auto next = current->next;
			delete current;
			current = next;
		}
	}

	std::atomic<bool>& GetActive() {
		return active_;
	}

	void Retire(T* ptr) {
		retired_list_node* node = new retired_list_node{ptr, nullptr};

		retired_list_node* old_head = retired_.load();
		do {
			node->next = old_head;
		} while (!retired_.compare_exchange_weak(old_head, node));
	}

	void Scan() {
		active_ = true;

		std::vector<T*> hazards;
		for (int i = 0; i < max_hp_; ++i) {
			auto ptr = hp_[i]->Get().load();
			if (ptr) {
				hazards.push_back(ptr);
			}
		}

		auto current = retired_.load();
		while (current) {
			bool in_use = false;
			for (auto it = hazards.begin(); !in_use && it != hazards.end(); ++it) {
				in_use = (*it == current->ptr);
			}

			if (!in_use) {
				retired_list_node* temp = current;
				current = current->next;
				retired_.compare_exchange_strong(temp, current);
				delete temp;
			}
			else {
				current = current->next;
			}
		}

		active_ = false;
	}

private:
	struct retired_list_node {
		T* ptr;
		retired_list_node* next;
	};

	unsigned int max_hp_;
	std::vector<HazardPointer<T>*> hp_;
	std::atomic<retired_list_node*> retired_;
	std::atomic<bool> active_;
};


#endif //HAZARD_POINTER_H
