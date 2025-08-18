#pragma once

#include <DynaVal.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using IrrelonEventCallback = std::function<void(const DynaVal&)>;
using IrrelonWildcardEventCallback = std::function<void(std::string eventName, const DynaVal&)>;

// EventEmitter class definition
class IrrelonEmitter {
public:
	// Register a listener for a specific namespace and event, with typed data
	void on(
		const std::string& eventName,
		const IrrelonEventCallback& listener
	) {
		auto wrappedListener = [listener](const std::string&, const DynaVal& data) {
			listener(data);
		};

		listeners[eventName].push_back(wrappedListener);
	}

	// Register a wildcard listener that handles any event data type
	void onWildcard(const IrrelonWildcardEventCallback& listener) {
		listeners["*"].push_back(listener);
	}

	// Emit an event with typed data for a specific namespace and event name
	void emit(const std::string& eventName, const DynaVal& data = {}) {
		// Emit to exact match listeners
		if (listeners.find(eventName) != listeners.end()) {
			callListeners(eventName, listeners[eventName], data);
		}

		// Emit to wildcard listeners
		if (listeners.find("*") != listeners.end()) {
			callListeners(eventName, listeners["*"], data);
		}
	}

private:
	// Map of event name to a list of wildcard listeners
	std::unordered_map<std::string, std::vector<IrrelonWildcardEventCallback>> listeners;

	// Helper function to call the listeners
	static void callListeners(
		const std::string& eventName,
		const std::vector<IrrelonWildcardEventCallback>& listenerList,
		const DynaVal& data
	) {
		for (const auto& listener : listenerList) {
			try {
				//Serial.println(("IrrelonEmitter::callListeners(" + eventName + ")").c_str());
				listener(eventName, data); // Call each listener
			} catch (const std::exception& e) {
				Serial.println("Exception in event handler: " + String(e.what()));
				Serial.print("Event Name: ");
				Serial.println(eventName.c_str());
			}
		}
	}
};