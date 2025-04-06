import doneProcess from "done:process.js";
import doneTestRunner from "done:test-runner.js";

function interpretArgv() {
    if (doneProcess.argv.includes("--test")) {
        doneTestRunner();
    }
}

interpretArgv();

// FIXME: currently every module has to have some export. This should be fixed. 
export default undefined;
