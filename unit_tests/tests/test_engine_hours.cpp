#include "pch.h"

#include "engine_hours.h"

static void runFor(EngineHours* dut, int seconds) {
	// slow callback is approx 20Hz, 0.5s steps are enough here
	for (int i = 0; i < seconds * 2; i++) {
		advanceTimeUs(500'000);
		dut->onSlowCallback();
	}
}

static EngineHours* setupEngineHours() {
	engineHoursLoseBackupForTest();
	engineHoursFlashCopy() = {};

	auto& dut = engine->module<EngineHours>().unmock();
	dut.resetForTest();
	dut.onSlowCallback();
	return &dut;
}

TEST(EngineHours, CountsOnlyWhileEngineTurns) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	auto dut = setupEngineHours();

	engine->rpmCalculator.setRpmValue(0);
	runFor(dut, 100);
	EXPECT_EQ(0u, dut->getSeconds(0));

	engine->rpmCalculator.setRpmValue(3000);
	runFor(dut, 100);
	for (size_t i = 0; i < ENGINE_HOURS_COUNT; i++) {
		EXPECT_NEAR(100, dut->getSeconds(i), 1);
	}
	EXPECT_NEAR(100 / 3600.0f, dut->getHours(4), 0.001f);

	engine->rpmCalculator.setRpmValue(0);
	runFor(dut, 100);
	EXPECT_NEAR(100, dut->getSeconds(4), 1);
}

TEST(EngineHours, SurvivesPowerCycleViaBackupRegisters) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	auto dut = setupEngineHours();

	engine->rpmCalculator.setRpmValue(3000);
	runFor(dut, 600);
	uint32_t before = dut->getSeconds(4);
	EXPECT_NEAR(600, before, 1);

	// power cycle, flash copy is zero but backup registers are valid
	dut->resetForTest();
	engine->rpmCalculator.setRpmValue(0);
	dut->onSlowCallback();
	EXPECT_EQ(before, dut->getSeconds(4));
}

TEST(EngineHours, FlashCopyWhenBackupLost) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	auto dut = setupEngineHours();

	engine->rpmCalculator.setRpmValue(3000);
	runFor(dut, 2000);
	// no flash write while running
	EXPECT_EQ(0u, dut->flashSaveCounter);

	engine->rpmCalculator.setRpmValue(0);
	dut->onSlowCallback();
	// more than threshold accumulated: flash save requested once engine stopped
	EXPECT_EQ(1u, dut->flashSaveCounter);
	uint32_t saved = dut->getSeconds(4);
	EXPECT_TRUE(engineHoursFlashCopy().isValid());
	EXPECT_EQ(saved, engineHoursFlashCopy().seconds[4]);

	// not again without more running
	runFor(dut, 10);
	EXPECT_EQ(1u, dut->flashSaveCounter);

	// run a bit more, below threshold: only backup registers are updated
	engine->rpmCalculator.setRpmValue(3000);
	runFor(dut, 100);
	engine->rpmCalculator.setRpmValue(0);
	runFor(dut, 1);
	EXPECT_EQ(1u, dut->flashSaveCounter);

	// backup battery lost + power cycle: restore last flash copy
	// (persistentState RAM copy is always current, simulate flash content)
	engineHoursFlashCopy().seconds[4] = saved;
	engineHoursFlashCopy().updateCrc();
	engineHoursLoseBackupForTest();
	dut->resetForTest();
	dut->onSlowCallback();
	EXPECT_EQ(saved, dut->getSeconds(4));
}

TEST(EngineHours, ResetSingleCounter) {
	EngineTestHelper eth(engine_type_e::TEST_ENGINE);
	auto dut = setupEngineHours();

	engine->rpmCalculator.setRpmValue(3000);
	runFor(dut, 300);

	// refused while running
	handleEngineHoursCommand(1);
	EXPECT_NEAR(300, dut->getSeconds(1), 1);

	engine->rpmCalculator.setRpmValue(0);
	handleEngineHoursCommand(1);
	EXPECT_EQ(0u, dut->getSeconds(1));
	EXPECT_NEAR(300, dut->getSeconds(0), 1);
	EXPECT_NEAR(300, dut->getSeconds(4), 1);
	// reset is persisted to flash immediately
	EXPECT_EQ(1u, dut->flashSaveCounter);
	EXPECT_EQ(0u, engineHoursFlashCopy().seconds[1]);

	// reset survives power cycle
	dut->resetForTest();
	dut->onSlowCallback();
	EXPECT_EQ(0u, dut->getSeconds(1));

	// invalid index is ignored
	handleEngineHoursCommand(7);
	EXPECT_NEAR(300, dut->getSeconds(0), 1);

	// explicit save
	handleEngineHoursCommand(ENGINE_HOURS_CMD_SAVE);
	EXPECT_EQ(2u, dut->flashSaveCounter);
}
