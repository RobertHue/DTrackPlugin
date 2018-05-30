#pragma once

#include "AutomationTest.h"

/////////////////////////////////////////////////////////////////////


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTrackPollThreadTest,	// The desired class name of the test. The macro will create a class of this name, e.g. FPlaceholderTest, in which your test can be implemented. 
	"DTrackPlugin.DTrackPollThread.from_dtrack_location Test",	// A string specifying the hierarchical test name that will appear in the UI
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter	// Test is suitable for running within the editor AND Engine Level Test.
)
