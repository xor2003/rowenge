// T105: minimal PE test-runner entry — replaces the upstream approach of
// embedding UnitTest::RunAllTests() + getchar() inside the MFC app's
// InitInstance (mfc/mig.cpp, TESTS define). Console-subsystem exe under
// Wine: stdout flows directly, exit code = failure count.
#include <UnitTest++.h>
#include <TestReporterStdout.h>

int main()
{
    return UnitTest::RunAllTests();
}
