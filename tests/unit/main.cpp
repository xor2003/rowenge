// Shared test-runner main for rowan-engine leaf tests (T101).
// Exit codes: 0 all pass, 1 failures, 2 zero checks ran (harness self-check).
#include "../harness.h"

int g_checks = 0;
int g_failures = 0;
int g_test_count = 0;
TestEntry g_tests[1024];

int main() {
    for (int i = 0; i < g_test_count; ++i) {
        std::printf("[ RUN  ] %s (%s:%d)\n", g_tests[i].name,
                    g_tests[i].file, g_tests[i].line);
        int before = g_failures;
        g_tests[i].fn();
        std::printf("[  %s ] %s\n", g_failures == before ? "PASS" : "FAIL",
                    g_tests[i].name);
    }
    std::printf("%d checks, %d failures, %d tests\n",
                g_checks, g_failures, g_test_count);
    if (g_checks == 0 || g_test_count == 0) {
        std::printf("HARNESS-ERROR: zero checks ran\n");
        return 2;
    }
    return g_failures ? 1 : 0;
}
