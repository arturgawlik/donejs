import doneProcess from "done:process.js";

function isValidModuleToRun(maybeModuleToRun) {
    if (typeof maybeModuleToRun !== 'string') {
        return false;
    }

    if (maybeModuleToRun.includes('--')) {
        // is some flag
        return false;
    }

    if (!maybeModuleToRun.includes('.') || !maybeModuleToRun.includes('/')) {
        // is path to file
        // TODO: here should be used fully flagged URL/path parser - when it will be created 
        return false;
    }

    return true;
}

export default function interpretArgv() {
    let donejsExec, moduleToRun, test, hasRestFlagsIndicator = false, nonDoneJsFlags = [];

    doneProcess.argv.forEach((argv, index) => {
        if (!hasRestFlagsIndicator) {
            // done.js flags
            if (index === 0) {
                // done exec path
                donejsExec = argv;
                return;
            }

            if (argv === '--') {
                // non done.js flags
                hasRestFlagsIndicator = true;
                return;
            }

            if (argv.startsWith('--')) {
                if (!!moduleToRun) {
                    throw new Error('Flags must be provided before module to run.');
                }

                if (argv === '--test') {
                    test = true;
                }
                return;
            }

            if (isValidModuleToRun(argv)) {
                // js module to run
                if (!!moduleToRun) {
                    throw new Error('Multiple modules to run are not allowed.');
                }

                moduleToRun = argv;
                return;
            }
        } else {
            // non done.js flags
            nonDoneJsFlags.push(argv);
        }
    });

    return {
        donejsExec,
        moduleToRun,
        doneJsFlags: {
            test
        },
        programFlags: nonDoneJsFlags
    }
}