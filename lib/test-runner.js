import doneConsole from "done:console.js";
import doneProcess from "done:process.js";

const kOutputPrefix = '[TEST RUNNER]: ';

export default function testRunner() {
    const globbuf = {};
    done.syscall.glob("./**/*.test.js", 1 << 3 /* GLOB_DOOFFS */, null, globbuf);

    const { gl_pathc: foundTestFilesCount, gl_pathv: foundTestFiles } = globbuf;
    doneConsole.log(`${kOutputPrefix}Running tests for ${foundTestFilesCount} test files.`);

    const donejsPath = doneProcess.argv[0];
    for (const testFile of foundTestFiles) {
        doneConsole.log(`${kOutputPrefix}Running tests for ${testFile} test file.`);
        const pid = new Int32Array(1);
        const argv = [donejsPath, testFile];
        const res = done.syscall.posix_spawn(pid, donejsPath, null, null, argv, null);
        // TODO: wait and check exit code of child process
    }
}