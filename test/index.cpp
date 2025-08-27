#include <string>
#include <unity.h>
#include <Irrelon/DynaVal.h>
#include <Irrelon/dynaLog.h>
#include "Irrelon/Emitter.h"

void test_event_emitter() {
	try {
		Irrelon::Emitter emitter;
		Irrelon::DynaVal records;
		emitter.on("event", [&records](const Irrelon::DynaVal& args) {
			records.set("Event fired");
		});

		emitter.emit("event");

		TEST_ASSERT_EQUAL_STRING("\"Event fired\"", records.toJson().c_str());
	} catch (const std::exception &e) {
		Irrelon::dynaLogLn("Exception", e.what());
		TEST_FAIL_MESSAGE(e.what());
	}
}

int main() {
	UNITY_BEGIN();
	RUN_TEST(test_event_emitter);
	UNITY_END();
}
