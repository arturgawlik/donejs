import doneJsConsole from "/home/artur-gawlik/git/donejs/lib/console.js";
import doneJsProcess from "/home/artur-gawlik/git/donejs/lib/process.js";
import doneJsTextDecoder from "/home/artur-gawlik/git/donejs/lib/text-decoder.js";

globalThis.console = doneJsConsole;
globalThis.process = doneJsProcess;
globalThis.exit = doneJsProcess.exit;
globalThis.TextDecoder = doneJsTextDecoder
