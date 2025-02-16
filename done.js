import doneJsConsole from "/home/artur-gawlik/git/donejs/lib/console.js";
import doneJsProcess from "/home/artur-gawlik/git/donejs/lib/process.js";

globalThis.console = doneJsConsole;
globalThis.process = doneJsProcess;
globalThis.exit = doneJsProcess.exit;
