import doneConsole from "done:console.js";
import doneProcess from "done:process.js";
import doneTextDecoder from "done:text-decoder.js";
import doneFetch from "done:fetch.js";
import doneInterpretFlags from "done:argv.js";
import "done:command.js";

globalThis.console = doneConsole;
globalThis.process = doneProcess;
globalThis.exit = doneProcess.exit;
globalThis.TextDecoder = doneTextDecoder
globalThis.fetch = doneFetch;

globalThis.done = {};
globalThis.done.interpretFlags = doneInterpretFlags;
