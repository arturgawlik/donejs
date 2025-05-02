import doneConsole from "done:console.js";
import doneProcess from "done:process.js";

const kOutputPrefix = '[TEST RUNNER]: ';

export default function testRunner() {
    const globbuf = {};
    done.syscall.glob("./**/*.test.js", 1 << 3 /* GLOB_DOOFFS */, null, globbuf);

    const { gl_pathc: foundTestFilesCount, gl_pathv: foundTestFiles } = globbuf;
    doneConsole.log(`${kOutputPrefix}Running tests for ${foundTestFilesCount} test files.`);

    const donejsPath = doneProcess.argv[0];
    let anyFailed = false;
    for (const testFile of foundTestFiles) {
        const pidBuffer = new Int32Array(1);
        const argv = [donejsPath, testFile];
        done.syscall.posix_spawn(pidBuffer, donejsPath, null, null, argv, null);
        const pid = pidBuffer[0];

        const statusBuffer = new Int32Array(1);
        done.syscall.waitpid(pid, statusBuffer, 0);
        const [status] = statusBuffer;
        const testPassed = done.syscall.macros.WEXITSTATUS(status) === 0
        if (!testPassed) {
            anyFailed = true;
        }
        doneConsole.log(`${kOutputPrefix}Test ${testFile} ${testPassed ? 'passed' : 'failed'}.`);
    }

    if (anyFailed) {
        doneConsole.log(`${kOutputPrefix}Tests failed. Exiting with 1 status code.`);
        doneProcess.exit(1)
    } else {
        doneConsole.log(`${kOutputPrefix}Tests passed.`);
    }
}
