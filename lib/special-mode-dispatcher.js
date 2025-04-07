import doneTestRunner from "done:test-runner.js";
import interpretArgv from "done:argv.js"

const { doneJsFlags } = interpretArgv();
if (doneJsFlags.test) {
    doneTestRunner();
}

// FIXME: currently every module has to have some export. This should be fixed. 
export default undefined;
