#include <Irrelon/Emitter.h>
#include <Irrelon/DynaVal.h>

void setup() {
	Irrelon::Emitter emitter;
	emitter.on("event", [](const Irrelon::DynaVal& args) {

	});

	emitter.emit("event");
}

void loop () {

}