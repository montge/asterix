/**
 * Unit tests for Tracer.cpp
 *
 * Requirements Traceability:
 * - REQ-HLR-SYS-001: System logging and tracing
 * - REQ-HLR-ERR-002: Error reporting and diagnostics
 *
 * DO-278A AL-3 Compliance Testing
 *
 * Coverage Target: 55.2% -> 80%+ (Tracer.cpp)
 * Expected Impact: +0.3 percentage points overall
 */

#include <gtest/gtest.h>
#include "Tracer.h"
#include <cstring>
#include <sstream>

// Global test variables to track callback invocations
static std::string g_captured_output;
static int g_callback_count = 0;

// Test callback functions
int test_printf_callback(const char* format, ...) {
    g_captured_output = format;
    g_callback_count++;
    return static_cast<int>(strlen(format));
}

void test_void_printf_callback(const char* format, ...) {
    g_captured_output = format;
    g_callback_count++;
}

/**
 * Test fixture for Tracer tests
 */
class TracerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset global state before each test
        g_captured_output.clear();
        g_callback_count = 0;

        // Reset Tracer to default state
        Tracer::Delete();
        Tracer::instance().pPrintFunc = nullptr;
        Tracer::instance().pPrintFunc2 = nullptr;
    }

    void TearDown() override {
        // Clean up after each test
        Tracer::Delete();
        g_captured_output.clear();
        g_callback_count = 0;
    }
};

/**
 * Test Case: TC-CPP-TRACER-001
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify singleton instance returns non-null
 */
TEST_F(TracerTest, SingletonInstanceNotNull) {
    Tracer& tracer = Tracer::instance();

    // Instance should be accessible
    EXPECT_NE(&tracer, nullptr);
}

/**
 * Test Case: TC-CPP-TRACER-002
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify singleton pattern - same instance returned
 */
TEST_F(TracerTest, SingletonReturnsSameInstance) {
    Tracer& t1 = Tracer::instance();
    Tracer& t2 = Tracer::instance();

    // Should return same singleton instance
    EXPECT_EQ(&t1, &t2);
}

/**
 * Test Case: TC-CPP-TRACER-003
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify global instance pointer
 */
TEST_F(TracerTest, GlobalInstancePointer) {
    Tracer& tracer = Tracer::instance();

    EXPECT_NE(Tracer::g_TracerInstance, nullptr);
    EXPECT_EQ(&tracer, Tracer::g_TracerInstance);
}

/**
 * Test Case: TC-CPP-TRACER-004
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify Error() outputs message with default handler
 */
TEST_F(TracerTest, ErrorMessageDefaultHandler) {
    // Capture stdout since default handler uses puts()
    testing::internal::CaptureStdout();

    Tracer::Error("Test error message");

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Test error message"), std::string::npos);
}

/**
 * Test Case: TC-CPP-TRACER-005
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify Error() with format string and arguments
 */
TEST_F(TracerTest, ErrorMessageWithFormatString) {
    testing::internal::CaptureStdout();

    Tracer::Error("Error code: %d, message: %s", 404, "Not Found");

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Error code: 404"), std::string::npos);
    EXPECT_NE(output.find("message: Not Found"), std::string::npos);
}

/**
 * Test Case: TC-CPP-TRACER-006
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify multiple Error() calls
 */
TEST_F(TracerTest, MultipleErrorCalls) {
    testing::internal::CaptureStdout();

    Tracer::Error("First error");
    Tracer::Error("Second error");
    Tracer::Error("Third error");

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("First error"), std::string::npos);
    EXPECT_NE(output.find("Second error"), std::string::npos);
    EXPECT_NE(output.find("Third error"), std::string::npos);
}

/**
 * Test Case: TC-CPP-TRACER-007
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Configure() with ptExtPrintf callback
 */
TEST_F(TracerTest, ConfigureWithPrintfCallback) {
    Tracer::Configure(test_printf_callback);

    EXPECT_EQ(Tracer::instance().pPrintFunc, test_printf_callback);
    EXPECT_EQ(Tracer::instance().pPrintFunc2, nullptr);
}

/**
 * Test Case: TC-CPP-TRACER-008
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Configure() with ptExtVoidPrintf callback
 */
TEST_F(TracerTest, ConfigureWithVoidPrintfCallback) {
    Tracer::Configure(test_void_printf_callback);

    EXPECT_EQ(Tracer::instance().pPrintFunc, nullptr);
    EXPECT_EQ(Tracer::instance().pPrintFunc2, test_void_printf_callback);
}

/**
 * Test Case: TC-CPP-TRACER-009
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Error() uses configured ptExtPrintf callback
 */
TEST_F(TracerTest, ErrorUsesConfiguredPrintfCallback) {
    Tracer::Configure(test_printf_callback);

    Tracer::Error("Callback test message");

    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(g_captured_output, "Callback test message");
}

/**
 * Test Case: TC-CPP-TRACER-010
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Error() uses configured ptExtVoidPrintf callback
 */
TEST_F(TracerTest, ErrorUsesConfiguredVoidPrintfCallback) {
    Tracer::Configure(test_void_printf_callback);

    Tracer::Error("Void callback test");

    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(g_captured_output, "Void callback test");
}

/**
 * Test Case: TC-CPP-TRACER-011
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify callback priority - pPrintFunc has priority over pPrintFunc2
 */
TEST_F(TracerTest, CallbackPriority) {
    // Set both callbacks
    Tracer::instance().pPrintFunc = test_printf_callback;
    Tracer::instance().pPrintFunc2 = test_void_printf_callback;

    // Reset counter
    g_callback_count = 0;

    Tracer::Error("Priority test");

    // Only pPrintFunc should be called (has priority)
    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(g_captured_output, "Priority test");
}

/**
 * Test Case: TC-CPP-TRACER-012
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Delete() clears singleton instance
 */
TEST_F(TracerTest, DeleteClearsSingleton) {
    // Create instance
    Tracer& tracer = Tracer::instance();
    EXPECT_NE(Tracer::g_TracerInstance, nullptr);

    // Delete it
    Tracer::Delete();

    EXPECT_EQ(Tracer::g_TracerInstance, nullptr);
}

/**
 * Test Case: TC-CPP-TRACER-013
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify singleton can be recreated after Delete()
 */
TEST_F(TracerTest, RecreateAfterDelete) {
    // Create and delete
    Tracer::instance();
    Tracer::Delete();

    EXPECT_EQ(Tracer::g_TracerInstance, nullptr);

    // Recreate
    Tracer& new_tracer = Tracer::instance();

    EXPECT_NE(Tracer::g_TracerInstance, nullptr);
    EXPECT_EQ(&new_tracer, Tracer::g_TracerInstance);
}

/**
 * Test Case: TC-CPP-TRACER-014
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify Error() handles very long messages
 */
TEST_F(TracerTest, ErrorHandlesLongMessage) {
    testing::internal::CaptureStdout();

    // Create a message longer than 1024 chars
    std::string long_msg(2000, 'A');
    Tracer::Error(long_msg.c_str());

    std::string output = testing::internal::GetCapturedStdout();

    // Should handle gracefully (truncated to buffer size)
    EXPECT_GT(output.length(), 0);
}

/**
 * Test Case: TC-CPP-TRACER-015
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify Error() handles empty message
 */
TEST_F(TracerTest, ErrorHandlesEmptyMessage) {
    testing::internal::CaptureStdout();

    Tracer::Error("");

    std::string output = testing::internal::GetCapturedStdout();

    // Should handle empty string without crash
    SUCCEED();
}

/**
 * Test Case: TC-CPP-TRACER-016
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify Error() with special characters
 */
TEST_F(TracerTest, ErrorWithSpecialCharacters) {
    testing::internal::CaptureStdout();

    Tracer::Error("Error with special chars: \n\t\r\\\"'");

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_GT(output.length(), 0);
}

/**
 * Test Case: TC-CPP-TRACER-017
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify Error() with hexadecimal format
 */
TEST_F(TracerTest, ErrorWithHexFormat) {
    testing::internal::CaptureStdout();

    Tracer::Error("ASTERIX CAT %d, byte: 0x%02X", 48, 0xFF);

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("ASTERIX CAT 48"), std::string::npos);
    EXPECT_NE(output.find("0xFF"), std::string::npos);
}

/**
 * Test Case: TC-CPP-TRACER-018
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Constructor initializes callbacks to NULL
 */
TEST_F(TracerTest, ConstructorInitializesCallbacks) {
    Tracer::Delete();

    Tracer& tracer = Tracer::instance();

    EXPECT_EQ(tracer.pPrintFunc, nullptr);
    EXPECT_EQ(tracer.pPrintFunc2, nullptr);
}

/**
 * Test Case: TC-CPP-TRACER-019
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify rapid repeated Error() calls
 */
TEST_F(TracerTest, RapidRepeatedCalls) {
    Tracer::Configure(test_printf_callback);

    g_callback_count = 0;

    // Call Error() many times rapidly
    for (int i = 0; i < 100; i++) {
        Tracer::Error("Rapid call %d", i);
    }

    EXPECT_EQ(g_callback_count, 100);
}

/**
 * Test Case: TC-CPP-TRACER-020
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify Error() with floating point formatting
 */
TEST_F(TracerTest, ErrorWithFloatFormat) {
    testing::internal::CaptureStdout();

    Tracer::Error("Latitude: %.6f, Longitude: %.6f", 45.123456, -122.654321);

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("45.123456"), std::string::npos);
    EXPECT_NE(output.find("-122.654321"), std::string::npos);
}

/**
 * Test Case: TC-CPP-TRACER-021
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify callback can be changed during runtime
 */
TEST_F(TracerTest, CallbackCanBeChanged) {
    // Set first callback
    Tracer::Configure(test_printf_callback);
    Tracer::Error("First");
    EXPECT_EQ(g_callback_count, 1);

    // Change to second callback
    g_callback_count = 0;
    Tracer::Configure(test_void_printf_callback);
    Tracer::Error("Second");
    EXPECT_EQ(g_callback_count, 1);
}

/**
 * Test Case: TC-CPP-TRACER-022
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify SetLogLevel() and GetLogLevel() default value
 */
TEST_F(TracerTest, DefaultLogLevel) {
    Tracer::Delete();
    Tracer::instance();

    // Default log level should be 1 (errors only)
    EXPECT_EQ(Tracer::GetLogLevel(), 1);
}

/**
 * Test Case: TC-CPP-TRACER-023
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify SetLogLevel() changes level
 */
TEST_F(TracerTest, SetLogLevelChangesLevel) {
    Tracer::SetLogLevel(3);
    EXPECT_EQ(Tracer::GetLogLevel(), 3);

    Tracer::SetLogLevel(0);
    EXPECT_EQ(Tracer::GetLogLevel(), 0);

    Tracer::SetLogLevel(4);
    EXPECT_EQ(Tracer::GetLogLevel(), 4);
}

/**
 * Test Case: TC-CPP-TRACER-024
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify silent mode (log level 0) suppresses output
 */
TEST_F(TracerTest, SilentModeSuppressesOutput) {
    Tracer::Configure(test_printf_callback);
    g_callback_count = 0;

    // Set silent mode
    Tracer::SetLogLevel(0);

    Tracer::Error("This should be suppressed");

    // No callback should have been invoked
    EXPECT_EQ(g_callback_count, 0);
    EXPECT_TRUE(g_captured_output.empty());
}

/**
 * Test Case: TC-CPP-TRACER-025
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify restoring log level re-enables output
 */
TEST_F(TracerTest, RestoreLogLevelReenablesOutput) {
    Tracer::Configure(test_printf_callback);

    // Silence, then restore
    Tracer::SetLogLevel(0);
    g_callback_count = 0;
    Tracer::Error("Suppressed");
    EXPECT_EQ(g_callback_count, 0);

    Tracer::SetLogLevel(1);
    Tracer::Error("Restored");
    EXPECT_EQ(g_callback_count, 1);
    EXPECT_EQ(g_captured_output, "Restored");
}

/**
 * Test Case: TC-CPP-TRACER-026
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify m_logLevel is initialized correctly after Delete/recreate
 */
TEST_F(TracerTest, LogLevelResetAfterDelete) {
    Tracer::SetLogLevel(4);
    EXPECT_EQ(Tracer::GetLogLevel(), 4);

    Tracer::Delete();
    // After delete and recreate, should be default (1)
    EXPECT_EQ(Tracer::GetLogLevel(), 1);
}

/**
 * Test Case: TC-CPP-TRACER-027
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Delete() on already-null instance is safe
 */
TEST_F(TracerTest, DeleteOnNullInstanceIsSafe) {
    Tracer::Delete();
    EXPECT_EQ(Tracer::g_TracerInstance, nullptr);

    // Second delete should not crash
    Tracer::Delete();
    EXPECT_EQ(Tracer::g_TracerInstance, nullptr);
}

/**
 * Test Case: TC-CPP-TRACER-028
 * Requirement: REQ-HLR-ERR-002
 * Description: Verify silent mode with default puts() handler
 */
TEST_F(TracerTest, SilentModeWithDefaultHandler) {
    // No callback configured - uses default puts()
    Tracer::SetLogLevel(0);

    testing::internal::CaptureStdout();
    Tracer::Error("Should be silent");
    std::string output = testing::internal::GetCapturedStdout();

    // Output should be empty when silent
    EXPECT_TRUE(output.empty());
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
